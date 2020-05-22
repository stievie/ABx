/**
 * Copyright 2017-2020 Stefan Ascher
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

#include <string>
#include <vector>
#include <iostream>
#include <abscommon/StringUtils.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/FileUtils.h>
#include <abscommon/Random.h>
#include <AB/DHKeys.hpp>
#include <abscommon/Utils.h>
#include <abscommon/Logo.h>
#include <sa/ArgParser.h>

static bool GenerateKeys(const std::string& outFile)
{
    Crypto::Random rnd;
    rnd.Initialize();
    Crypto::DHKeys keys;
    keys.GenerateKeys();
    return keys.SaveKeys(outFile);
}

static void ShowLogo()
{
    std::cout << "This is AB Key Generator" << std::endl;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
    std::cout << "(C) 2017-" << CURRENT_YEAR << std::endl;

    std::cout << std::endl;

    std::cout << AB_CONSOLE_LOGO << std::endl;

    std::cout << std::endl;
}

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << "This program generates a Diffie Hellman key pair for the server." << std::endl << std::endl;
    std::cout << sa::arg_parser::get_help("keygen", _cli);
}

int main(int argc, char** argv)
{
    ShowLogo();
    const std::string exeFile = Utils::GetExeName();
    const std::string path = Utils::ExtractFileDir(exeFile);

    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none },
        { "file", { "-o", "--output-file" }, "Output file", false, true, sa::arg_parser::option_type::string },
        { "force", { "-f", "--force" }, "Overwrite existing file without asking", false, false, sa::arg_parser::option_type::none }
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
        std::cout << "Type `keygen -h` for help." << std::endl;
        return EXIT_FAILURE;
    }

    std::string cfgFile = Utils::ConcatPath(path, "abserv.lua");
    IO::SimpleConfigManager cfg;
    if (!cfg.Load(cfgFile))
    {
        std::cerr << "Failed to load config file " << cfgFile << std::endl;
        return EXIT_FAILURE;
    }
    std::string keyFile = cfg.GetGlobalString("server_keys", "");
    if (keyFile.empty())
        keyFile = Utils::ConcatPath(path, "abserver.dh");
    keyFile = sa::arg_parser::get_value<std::string>(parsedArgs, "file", keyFile);

    if (Utils::FileExists(keyFile))
    {
        if (!sa::arg_parser::get_value<bool>(parsedArgs, "force", false))
        {
            std::cout << "Overwrite existing file " << keyFile << " (y/n)? ";
            std::string answer;
            if (!std::getline(std::cin, answer))
                return EXIT_FAILURE;
            if (answer.compare("y") != 0)
            {
                std::cout << "Aborted" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    if (!GenerateKeys(keyFile))
    {
        std::cerr << "Error generating keys: " << keyFile << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Generated new keys: " << keyFile << std::endl;
    return EXIT_SUCCESS;
}

