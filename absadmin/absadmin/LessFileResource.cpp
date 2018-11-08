#include "stdafx.h"
#include "LessFileResource.h"
#include "Application.h"
#include "Logger.h"
#include "FileUtils.h"
#include "ContentTypes.h"
#include "Subsystems.h"
#include "StringUtils.h"
#include "Profiler.h"

#include <less/less/LessTokenizer.h>
#include <less/less/LessParser.h>
#include <less/css/CssWriter.h>
#include <less/css/CssPrettyWriter.h>
#include <less/stylesheet/Stylesheet.h>
#include <less/css/IOException.h>
#include <less/lessstylesheet/LessStylesheet.h>

namespace Resources {

static char* path_create_relative(const char* path, const char* relative) {
    const char* p_root;
    const char* r_root;
    size_t desc_n;
    char* ret, *ret_p;

    // strip common directories
    while ((p_root = strchr(path, '/')) != NULL &&
        (r_root = strchr(relative, '/')) != NULL &&
        p_root - path == r_root - relative &&
        strncmp(path, relative, p_root - path) == 0) {
        path = p_root + 1;
        relative = r_root + 1;
    }

    // count how many folders to go up.
    for (desc_n = 0; relative != NULL; desc_n++) {
        relative = strchr(relative + 1, '/');
    }
    desc_n--;

    ret = new char[strlen(path) + 3 * desc_n + 1];
    ret_p = ret;
    while (desc_n > 0) {
        memcpy(ret_p, "../", 3);
        ret_p += 3;
        desc_n--;
    }

    strcpy(ret_p, path);
    return ret;
}

static bool parseInput(LessStylesheet &stylesheet,
    istream &in,
    const char* source,
    std::list<const char*> &sources,
    std::list<const char*> &includePaths) {
    std::list<const char*>::iterator i;

    LessTokenizer tokenizer(in, source);
    LessParser parser(tokenizer, sources);
    parser.includePaths = &includePaths;

    try {
        parser.parseStylesheet(stylesheet);
    }
    catch (ParseException* e) {

        LOG_ERROR << e->getSource() << ": Line " << e->getLineNumber() << ", Column " <<
            e->getColumn() << " Parse Error: " << e->what() << endl;

        return false;
    }
    catch (exception* e) {
        LOG_ERROR << " Error: " << e->what() << endl;
        return false;
    }

    return true;
}

static bool processStylesheet(const LessStylesheet &stylesheet,
    Stylesheet &css) {
    ProcessingContext context;

    try {
        stylesheet.process(css, &context);

    }
    catch (ParseException* e) {

        LOG_ERROR << e->getSource() << ": Line " << e->getLineNumber() << ", Column " <<
            e->getColumn() << " Parse Error: " << e->what() << endl;
        return false;

    }
    catch (LessException* e) {

        LOG_ERROR << e->getSource() << ": Line " << e->getLineNumber() << ", Column " <<
            e->getColumn() << " Error: " << e->what() << endl;
        return false;

    }
    catch (exception* e) {

        LOG_ERROR << "Error: " << e->what() << endl;
        return false;
    }
    return true;
}

static void writeOutput(Stylesheet &css,
    const char* output,
    bool formatoutput,
    const char* rootpath,
    std::list<const char*> &sources,
    const char* sourcemap_file,
    const char* sourcemap_rootpath,
    const char* sourcemap_basepath,
    const char* sourcemap_url) {
    CssWriter* writer;
    ostream* sourcemap_s = NULL;
    SourceMapWriter* sourcemap = NULL;

    std::list<const char*> relative_sources;
    std::list<const char*>::iterator it;
    size_t bp_l = 0;

    if (sourcemap_basepath != NULL)
        bp_l = strlen(sourcemap_basepath);

    ofstream out(output);

    if (sourcemap_file != NULL) {
        for (it = sources.begin(); it != sources.end(); it++) {
            if (sourcemap_basepath == NULL) {
                relative_sources.push_back(path_create_relative(*it, sourcemap_file));
            }
            else if (strncmp(*it, sourcemap_basepath, bp_l) == 0) {
                relative_sources.push_back(*it + bp_l);
            }
            else {
                relative_sources.push_back(*it);
            }
        }

        sourcemap_s = new ofstream(sourcemap_file);
        sourcemap = new SourceMapWriter(*sourcemap_s,
            sources,
            relative_sources,
            path_create_relative(output, sourcemap_file),
            sourcemap_rootpath);

        writer = formatoutput ? new CssPrettyWriter(out, *sourcemap) :
            new CssWriter(out, *sourcemap);
    }
    else {
        writer = formatoutput ? new CssPrettyWriter(out) :
            new CssWriter(out);
    }
    writer->rootpath = rootpath;

    css.write(*writer);

    if (sourcemap != NULL) {
        if (sourcemap_url != NULL)
            writer->writeSourceMapUrl(sourcemap_url);
        else
            writer->writeSourceMapUrl(path_create_relative(sourcemap_file, output));

        sourcemap->close();
        delete sourcemap;
        if (sourcemap_s != NULL)
            delete sourcemap_s;
    }

    delete writer;
}

void LessFileResource::CompileFile(const std::string& source, const std::string& dest)
{
    Stylesheet css;
    LessStylesheet stylesheet;
    std::list<const char*> includePaths;
    std::list<const char*> sources;
    includePaths.push_back("");

    ifstream in = std::ifstream(source);
    if (in.fail() || in.bad())
    {
        throw std::invalid_argument("File not found");
    }

    sources.push_back(source.c_str());

    if (parseInput(stylesheet, in, source.c_str(), sources, includePaths)) {
        if (!processStylesheet(stylesheet, css))
            throw std::invalid_argument("Parsing error");

        writeOutput(css,
            dest.c_str(),
            false,
            nullptr,
            sources,
            nullptr,
            nullptr,
            nullptr,
            nullptr);
    }
}

void LessFileResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    const std::string& root = Application::Instance->GetRoot();

    try
    {
        auto web_root_path = fs::canonical(root);
        auto path = fs::canonical(web_root_path / request_->path);
        std::string output = Utils::ChangeFileExt(path.string(), ".css");

        bool createNew = false;
        if (Utils::FileExists(output))
        {
            auto sourceFt = fs::last_write_time(path);
            auto outputFt = fs::last_write_time(output);
            createNew = sourceFt > outputFt;
        }
        else
            createNew = true;

        if (createNew)
        {
            AB_PROFILE;
            CompileFile(path.string(), output);
        }

        request_->path = Utils::ChangeFileExt(request_->path, ".css");
        FileResource::Render(response);
    }
    catch (const std::exception&)
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found " + request_->path);
    }

}

}
