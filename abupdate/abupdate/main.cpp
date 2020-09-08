#include <iostream>
#include <sa/ArgParser.h>
#include <filesystem>
#include <sa/time.h>
#include <memory>
#include <absync/LocalBackend.h>
#include <absync/HttpRemoteBackend.h>
#include <absync/Synchronizer.h>

namespace fs = std::filesystem;

static std::unique_ptr<Sync::FileLocalBackend> localBackend;
static std::unique_ptr<Sync::HttpRemoteBackend> remoteBackend;

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << sa::arg_parser::get_help("abupdate", _cli, "Client updater");
    std::cout << std::endl;
}

static bool ProcessFile(const std::string& filename)
{
    std::cout << "Processing file " << filename << std::endl;
    Sync::Synchronizer sync(*localBackend, *remoteBackend);
    if (!sync.Synchronize(filename))
    {
        std::cerr << "Error synchronizing " << filename;
        return false;
    }
    if (sync.IsDifferent())
    {
        std::cout << "  Copied " << sync.GetCopied() << " bytes" << std::endl;
        std::cout << "  Downloaded " << sync.GetDownloaded() << " bytes" << std::endl;
        std::cout << "  Download savings " << 100 - (100.0 / sync.GetFilesize() * sync.GetDownloaded()) << "%" << std::endl;
    }
    else
    {
        std::cout << "  File is up to date" << std::endl;
    }

    return true;
}

static bool ProcessEntry(const fs::directory_entry& p)
{
    if (p.is_directory())
        return true;
    if (p.path().extension().string() != ".pak")
        return true;

    const std::string filename = p.path().string();
    return ProcessFile(filename);
}

template<typename T>
static bool ProcessDirectory(const T& iterator)
{
    bool result = true;
    for (const auto& p : iterator)
    {
        if (!ProcessEntry(p))
            result = false;
    }
    return result;
}

int main(int argc, char** argv)
{
    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "--help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none },
        { "recursive", { "-R", "--recursive" }, "Processs also subdirectores", false, false, sa::arg_parser::option_type::none },
        { "host", { "-H", "--server-host" }, "Server host", true, true, sa::arg_parser::option_type::string },
        { "port", { "-P", "--server-port" }, "Server port", true, true, sa::arg_parser::option_type::integer },
        { "account", { "-a", "--account" }, "Account", true, true, sa::arg_parser::option_type::string },
        { "token", { "-t", "--token" }, "Auth token to login", true, true, sa::arg_parser::option_type::string },
    } };

    sa::arg_parser::values parsedArgs;
    sa::arg_parser::result cmdres = sa::arg_parser::parse(argc, argv, _cli, parsedArgs);
    auto val = sa::arg_parser::get_value<bool>(parsedArgs, "help");
    if (val.has_value() && val.value())
    {
        ShowHelp(_cli);
        return EXIT_SUCCESS;
    }

    if (!cmdres)
        return EXIT_FAILURE;

    const std::string host = sa::arg_parser::get_value<std::string>(parsedArgs, "host", "");
    if (host.empty())
    {
        std::cerr << "No host" << std::endl;
        return EXIT_FAILURE;
    }
    uint16_t port = sa::arg_parser::get_value<uint16_t>(parsedArgs, "port", 0u);
    if (port == 0)
    {
        std::cerr << "No port" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string account = sa::arg_parser::get_value<std::string>(parsedArgs, "account", "");
    if (account.empty())
    {
        std::cerr << "No Account" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string token = sa::arg_parser::get_value<std::string>(parsedArgs, "token", "");
    if (token.empty())
    {
        std::cerr << "No Auth token" << std::endl;
        return EXIT_FAILURE;
    }

    httplib::Headers headers;
    std::stringstream ss;
    ss << account << token;
    httplib::Headers header = {
        { "Auth", ss.str() }
    };
    remoteBackend = std::make_unique<Sync::HttpRemoteBackend>(host, port, headers);
    localBackend = std::make_unique<Sync::FileLocalBackend>();

    auto recursive = sa::arg_parser::get_value<bool>(parsedArgs, "recursive", false);
    auto dirarg = sa::arg_parser::get_value<std::string>(parsedArgs, "0");
    fs::path dir = dirarg.has_value() ? dirarg.value() : fs::current_path();

    std::cout << "Directory " << dir.string() << std::endl;
    sa::time::timer timer;

    bool result = true;
    if (recursive)
        result = ProcessDirectory(fs::recursive_directory_iterator(dir));
    else
        result = ProcessDirectory(fs::directory_iterator(dir));

    std::cout << "Took " << timer.elapsed_seconds() << " seconds" << std::endl;

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
