// keygen.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <string>
#include <vector>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "StringUtils.h"
#include "SimpleConfigManager.h"
#include "FileUtils.h"
#include "Random.h"
#include <AB/DHKeys.hpp>
#include "Utils.h"

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

    std::cout << "##########  ######  ######" << std::endl;
    std::cout << "    ##          ##  ##" << std::endl;
    std::cout << "    ##  ######  ##  ##" << std::endl;
    std::cout << "    ##  ##      ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;

    std::cout << std::endl;
}

static void ShowHelp()
{
    std::cout << "This program generates a Diffie Hellman key pair for the server." << std::endl << std::endl;
    std::cout << "keygen [<options>]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  -o <file>: Out file. Default as given in abserv.lua" << std::endl;
    std::cout << "  -h: Show this help" << std::endl;
}

int main(int argc, char** argv)
{
    ShowLogo();
    std::string exeFile;
#ifdef _WIN32
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    exeFile = std::string(buff);
#else
    char buff[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buff, PATH_MAX);
    exeFile_ = std::string(buff, (count > 0) ? count : 0);
#endif
    std::string path = Utils::ExtractFileDir(exeFile);
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    if (Utils::GetCommandLineValue(args, "-h"))
    {
        ShowHelp();
        return EXIT_SUCCESS;
    }

    std::string cfgFile = path + "/abserv.lua";
    IO::SimpleConfigManager cfg;
    if (!cfg.Load(cfgFile))
    {
        std::cout << "Failed too load config file " << cfgFile << std::endl;
        return EXIT_FAILURE;
    }
    std::string keyFile = cfg.GetGlobalString("server_keys", "");
    if (keyFile.empty())
        keyFile = Utils::AddSlash(path) + "abserver.dh";
    Utils::GetCommandLineValue(args, "-o", keyFile);

    if (!GenerateKeys(keyFile))
    {
        std::cout << "Error generating keys: " << keyFile << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Generated new keys: " << keyFile << std::endl;
    return EXIT_SUCCESS;
}

