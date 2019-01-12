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
    std::string cfgFile = path + "/abserv.lua";
    IO::SimpleConfigManager cfg;
    if (!cfg.Load(cfgFile))
    {
        std::cout << "Failed too load config file " << cfgFile << std::endl;
        return EXIT_FAILURE;
    }
    std::string keyFile = cfg.GetGlobal("server_keys", "");
    if (keyFile.empty())
        keyFile = Utils::AddSlash(path) + "abserver.dh";

    if (!GenerateKeys(keyFile))
    {
        std::cout << "Error generating keys: " << keyFile << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Generated new keys: " << keyFile << std::endl;
    return EXIT_SUCCESS;
}

