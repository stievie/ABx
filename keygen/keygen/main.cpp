#include "stdafx.h"
#include <string>
#include <vector>
#include <iostream>
#include "StringUtils.h"
#include "SimpleConfigManager.h"
#include "FileUtils.h"
#include "Random.h"
#include <AB/DHKeys.hpp>
#include "Utils.h"
#include "Logo.h"
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
    std::cout << "(C) 2017-2019" << std::endl;

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
        { "file", { "-o", "--output-file" }, "Output file", false, true, sa::arg_parser::option_type::string }
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

    std::string cfgFile = path + "/abserv.lua";
    IO::SimpleConfigManager cfg;
    if (!cfg.Load(cfgFile))
    {
        std::cout << "Failed to load config file " << cfgFile << std::endl;
        return EXIT_FAILURE;
    }
    std::string keyFile = cfg.GetGlobalString("server_keys", "");
    if (keyFile.empty())
        keyFile = Utils::AddSlash(path) + "abserver.dh";
    keyFile = sa::arg_parser::get_value<std::string>(parsedArgs, "file", keyFile);

    if (!GenerateKeys(keyFile))
    {
        std::cout << "Error generating keys: " << keyFile << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Generated new keys: " << keyFile << std::endl;
    return EXIT_SUCCESS;
}

