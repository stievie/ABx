/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <sa/ArgParser.h>
#include <filesystem>
#include <sa/time.h>
#include <memory>
#include <absync/Synchronizer.h>
#include <absync/Hash.h>
#include <sa/StringTempl.h>
#include <absync/Updater.h>
#include <ProtocolLogin.h>
#include <Client.h>
#include <AB/DHKeys.hpp>
#include <asio.hpp>
#include <sa/Process.h>
#include <AB/CommonConfig.h>
#include "Platform.h"

namespace fs = std::filesystem;

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
    login->SetErrorCallback([](Client::ConnectionError error, const std::error_code&) {
        std::cerr << "Network error: " << Client::Client::GetNetworkErrorMessage(error) << std::endl;
    });
    login->SetProtocolErrorCallback([](AB::ErrorCodes error) {
        std::cerr << "Protocol error " << Client::Client::GetProtocolErrorMessage(error) << std::endl;
    });
    login->Login(host, port, username, password,
        [&](const std::string& accountUuid, const std::string& authToken, AB::Entities::AccountType)
    {
        accUuid = accountUuid;
        atoken = authToken;
    },
        [](const AB::Entities::CharList&)
    {
        // Nothing to do here
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
    std::cout << "This is AB Client updater";
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
    std::cout << "Running on " << System::GetPlatform() << std::endl << std::endl;
    std::cout << "(C) 2017-" << CURRENT_YEAR << std::endl;
    std::cout << std::endl;

    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "--help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none },
        { "host", { "-H", "--server-host" }, "Server host", true, true, sa::arg_parser::option_type::string },
        { "port", { "-P", "--server-port" }, "Server port", true, true, sa::arg_parser::option_type::integer },
        { "account", { "-a", "--account-key" }, "Account", false, true, sa::arg_parser::option_type::string },
        { "token", { "-t", "--auth-token" }, "Auth token to login", false, true, sa::arg_parser::option_type::string },
        { "user", { "-u", "--username" }, "Username to login", false, true, sa::arg_parser::option_type::string },
        { "pass", { "-p", "--password" }, "Password to login", false, true, sa::arg_parser::option_type::string },
        { "pattern", { "-m", "--match-pattern" }, "Filename pattern (default *.pak)", false, true, sa::arg_parser::option_type::string },
        { "run", { "-r", "--run" }, "Run program after update", false, true, sa::arg_parser::option_type::string },
        { "directory", { }, "Directory to update (default current directory)", false, true, sa::arg_parser::option_type::string },
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
    {
        std::cout << cmdres << std::endl;
        ShowHelp(_cli);
        return EXIT_FAILURE;
    }

    // File or login server
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
    std::string indexFile = sa::StringToLower(System::GetPlatform()) + "/_files_";
    Sync::Updater updater(host, port, authHeader, dir.string(), indexFile);
    updater.onError = [](Sync::Updater::ErrorType type, const char* message)
    {
        if (type == Sync::Updater::ErrorType::Remote)
            std::cerr << "HTTP Error ";
        else if (type == Sync::Updater::ErrorType::Local)
            std::cerr << "File Error ";
        std::cerr << message << std::endl;
    };
    updater.onProcessFile_ = [](size_t, size_t, const std::string& filename) -> bool
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
        std::cerr << "Error synchronizing " << filename << std::endl;
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

    std::cout << "Updating " << dir.string() << "..." << std::endl;
    sa::time::timer timer;
    bool result = updater.Execute();
    std::cout << "Took " << timer.elapsed_seconds() << " seconds" << std::endl;
    if (result)
    {
        auto run = sa::arg_parser::get_value<std::string>(parsedArgs, "run");
        if (run.has_value())
        {
            std::stringstream ss;
            ss << "\"" << run.value() << "\"";
            sa::Process::Run(ss.str());
        }
    }

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
