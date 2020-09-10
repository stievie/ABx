#include <iostream>
#include <sa/ArgParser.h>
#include <filesystem>
#include <sa/time.h>
#include <memory>
#include <absync/LocalBackend.h>
#include <absync/HttpRemoteBackend.h>
#include <absync/Synchronizer.h>
#include <absync/Hash.h>
#include <sa/StringTempl.h>
#include <absync/Updater.h>
#include <libabclient/ProtocolLogin.h>
#include <AB/DHKeys.hpp>
#include <asio.hpp>

namespace fs = std::filesystem;

static std::unique_ptr<Sync::FileLocalBackend> localBackend;
static std::unique_ptr<Sync::HttpRemoteBackend> remoteBackend;
static std::vector<std::string> patterns = { "*.pak" };

static std::string host;
static uint16_t port;
static std::string account;
static std::string token;
static std::string username;
static std::string password;

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << sa::arg_parser::get_help("abupdate", _cli, "Client updater");
    std::cout << std::endl;
}

static std::string GetAuthHeader()
{
    if (!account.empty() && !token.empty())
        return account + token;

    if (username.empty() && password.empty())
    {
        std::cerr << "Username and/or password missing" << std::endl;
        return "";
    }

    Crypto::DHKeys keys;
    keys.GenerateKeys();
    asio::io_service ioService;

    std::string accUuid;
    std::string atoken;
    std::shared_ptr<Client::ProtocolLogin> login = std::make_shared<Client::ProtocolLogin>(keys, ioService);
    login->SetErrorCallback([](Client::ConnectionError, const std::error_code&) {
        std::cerr << "Network error" << std::endl;
    });
    login->SetProtocolErrorCallback([](AB::ErrorCodes) {
        std::cerr << "Protocol error" << std::endl;
    });
    login->Login(host, port, username, password,
        [&](const std::string& accountUuid, const std::string& authToken, AB::Entities::AccountType)
    {
        accUuid = accountUuid;
        atoken = authToken;
    },
        [](const AB::Entities::CharList&)
    {
    });
    ioService.run();
    if (accUuid.empty() || atoken.empty())
        return "";

    port = login->filePort_;
    if (!login->fileHost_.empty())
        host = login->fileHost_;

    return accUuid + atoken;
}

int main(int argc, char** argv)
{
    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "--help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none },
        { "host", { "-H", "--server-host" }, "Server host", true, true, sa::arg_parser::option_type::string },
        { "port", { "-P", "--server-port" }, "Server port", true, true, sa::arg_parser::option_type::integer },
        { "account", { "-a", "--account-key" }, "Account", false, true, sa::arg_parser::option_type::string },
        { "token", { "-t", "--auth-token" }, "Auth token to login", false, true, sa::arg_parser::option_type::string },
        { "user", { "-u", "--username" }, "Username to login", false, true, sa::arg_parser::option_type::string },
        { "pass", { "-p", "--password" }, "Password to login", false, true, sa::arg_parser::option_type::string },
        { "pattern", { "-m", "--match-pattern" }, "Filename pattern (default *.pak)", false, true, sa::arg_parser::option_type::string },
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

    host = sa::arg_parser::get_value<std::string>(parsedArgs, "host", "");
    if (host.empty())
    {
        std::cerr << "No host" << std::endl;
        return EXIT_FAILURE;
    }
    port = sa::arg_parser::get_value<uint16_t>(parsedArgs, "port", 0u);
    if (port == 0)
    {
        std::cerr << "No port" << std::endl;
        return EXIT_FAILURE;
    }

    auto pattern = sa::arg_parser::get_value<std::string>(parsedArgs, "pattern");
    if (pattern.has_value())
    {
        patterns = sa::Split(pattern.value(), ";", false, false);
    }

    account = sa::arg_parser::get_value<std::string>(parsedArgs, "account", "");
    token = sa::arg_parser::get_value<std::string>(parsedArgs, "token", "");
    username = sa::arg_parser::get_value<std::string>(parsedArgs, "user", "");
    password = sa::arg_parser::get_value<std::string>(parsedArgs, "pass", "");

    if ((account.empty() || token.empty()) && (username.empty() || password.empty()))
    {
        std::cout << "Enter username: ";
        std::getline(std::cin, username);
        std::cout << "Enter password: ";
        std::getline(std::cin, password);
    }
    const std::string authHeader = GetAuthHeader();
    if (authHeader.empty())
    {
        std::cerr << "Unable to login" << std::endl;
        return EXIT_FAILURE;
    }

    auto dirarg = sa::arg_parser::get_value<std::string>(parsedArgs, "0");
    fs::path dir = dirarg.has_value() ? fs::path(dirarg.value()) : fs::current_path();
    Sync::Updater updater(host, port, authHeader, dir.string());
    updater.onProcessFile_ = [](const std::string filename) -> bool
    {
        bool match = false;
        for (const auto& pattern : patterns)
        {
            if (sa::PatternMatch<char>(filename, pattern))
            {
                match = true;
                break;
            }
        }
        if (match)
        {
            std::cout << "Processing file " << filename << std::endl;
        }
        return match;
    };
    updater.onFailure_ = [](const std::string& filename)
    {
        std::cerr << "Error synchronizing " << filename;
    };
    updater.onDoneFile_ = [](const std::string&, bool different, size_t downloaded, size_t copied, int savings)
    {
        if (different)
        {
            std::cout << "  Copied " << copied << " bytes" << std::endl;
            std::cout << "  Downloaded " << downloaded << " bytes" << std::endl;
            std::cout << "  Download savings " << savings << "%" << std::endl;
        }
        else
        {
            std::cout << "  File is up to date" << std::endl;
        }
    };
    updater.onProgress_ = [](size_t, size_t, size_t value, size_t max)
    {
        std::cout << '\r';
        std::cout << "[" << value << "/" << max << "]";
        if (value == max)
            std::cout << " done" << std::endl;
    };

    sa::time::timer timer;
    bool result = updater.Execute();
    std::cout << "Took " << timer.elapsed_seconds() << " seconds" << std::endl;

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
