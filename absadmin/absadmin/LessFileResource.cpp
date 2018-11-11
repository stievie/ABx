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

#include "DebugNew.h"

namespace Resources {

static bool ParseInput(LessStylesheet& stylesheet,
    istream& in,
    const std::string& source,
    std::list<const char*>& sources,
    std::list<const char*>& includePaths)
{
    LessTokenizer tokenizer(in, source.c_str());
    LessParser parser(tokenizer, sources);
    parser.includePaths = &includePaths;

    try
    {
        parser.parseStylesheet(stylesheet);
    }
    catch (ParseException* e)
    {

        LOG_ERROR << e->getSource() << ": Line " << e->getLineNumber() << ", Column " <<
            e->getColumn() << " Parse Error: " << e->what() << endl;
        return false;
    }
    catch (exception* e)
    {
        LOG_ERROR << " Error: " << e->what() << endl;
        return false;
    }

    return true;
}

static bool ProcessStylesheet(const LessStylesheet& stylesheet, Stylesheet& css)
{
    ProcessingContext context;
    try
    {
        stylesheet.process(css, &context);
    }
    catch (ParseException* e)
    {

        LOG_ERROR << e->getSource() << ": Line " << e->getLineNumber() << ", Column " <<
            e->getColumn() << " Parse Error: " << e->what() << endl;
        return false;

    }
    catch (LessException* e)
    {

        LOG_ERROR << e->getSource() << ": Line " << e->getLineNumber() << ", Column " <<
            e->getColumn() << " Error: " << e->what() << endl;
        return false;

    }
    catch (exception* e)
    {

        LOG_ERROR << "Error: " << e->what() << endl;
        return false;
    }
    return true;
}

static void WriteOutput(Stylesheet& css, const std::string& output, const char* rootpath)
{
    ofstream out(output);

    CssWriter writer(out);
    writer.rootpath = rootpath;

    css.write(writer);
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
        throw std::invalid_argument("File not found");

    sources.push_back(source.c_str());

    if (ParseInput(stylesheet, in, source, sources, includePaths))
    {
        if (!ProcessStylesheet(stylesheet, css))
            throw std::invalid_argument("Parsing error");

        WriteOutput(css, dest, nullptr);
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
            std::lock_guard<std::mutex> lock(mutex_);
            AB_PROFILE;
            CompileFile(path.string(), output);
        }

        request_->path = Utils::ChangeFileExt(request_->path, ".css");
        FileResource::Render(response);
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR << ex.what() << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found " + request_->path);
    }

}

}
