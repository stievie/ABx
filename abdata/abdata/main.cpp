// abdata.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include "Server.h"
#include "Logger.h"
#include "Database.h"
#include "ConfigManager.h"
#include "Version.h"
#include <signal.h>     /* signal, raise, sig_atomic_t */
#include <functional>

#if defined(_MSC_VER) && defined(_DEBUG)
#   define CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#endif

int gPort = 0;
size_t gMaxSize = 0;
std::string gConfigFile = "";

namespace {
std::function<void(int)> shutdown_handler;
void signal_handler(int signal)
{
    shutdown_handler(signal);
}
} // namespace

static void ShowLogo()
{
    std::cout << "This is " << SERVER_PRODUCT_NAME << std::endl;
    std::cout << "Version " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR <<
        " (" << __DATE__ << " " << __TIME__ << ")";
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
    std::cout << "(C) 2017-" << SERVER_YEAR << std::endl;
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
    std::cout << "abdata <option>" << std::endl;
}

static bool ParseCommandline(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        const std::string arg(argv[i]);
        if (arg.compare("-port") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                gPort = std::atoi(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -port" << std::endl;
        }
        else if (arg.compare("-maxsize") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                gMaxSize = std::atoi(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -maxsize" << std::endl;
        }
        else if (arg.compare("-log") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                IO::Logger::logDir_ = std::string(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -log" << std::endl;
        }
        else if (arg.compare("-conf") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                gConfigFile = std::string(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -log" << std::endl;
        }
        else if (arg.compare("-dbdriver") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                DB::Database::driver_ = std::string(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -dbdriver" << std::endl;
        }
        else if (arg.compare("-dbhost") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                DB::Database::dbHost_ = std::string(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -dbhost" << std::endl;
        }
        else if (arg.compare("-dbname") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                DB::Database::dbName_ = std::string(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -dbname" << std::endl;
        }
        else if (arg.compare("-dbuser") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                DB::Database::dbUser_ = std::string(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -dbuser" << std::endl;
        }
        else if (arg.compare("-dbpass") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                DB::Database::dbPass_ = std::string(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -dbpass" << std::endl;
        }
        else if (arg.compare("-dbport") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                DB::Database::dbPort_ = (uint16_t)std::atoi(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -dbport" << std::endl;
        }
    }
    return true;
}

static void LoadConfig()
{
    gPort = static_cast<int>(ConfigManager::Instance.GetGlobal("data_port", gPort));
    gMaxSize = ConfigManager::Instance.GetGlobal("max_size", gMaxSize);
    if (IO::Logger::logDir_.empty())
        IO::Logger::logDir_ = ConfigManager::Instance.GetGlobal("log_dir", "");
    if (DB::Database::driver_.empty())
        DB::Database::driver_ = ConfigManager::Instance.GetGlobal("db_driver", "");
    if (DB::Database::dbHost_.empty())
        DB::Database::dbHost_ = ConfigManager::Instance.GetGlobal("db_host", "");
    if (DB::Database::dbName_.empty())
        DB::Database::dbName_ = ConfigManager::Instance.GetGlobal("db_name", "");
    if (DB::Database::dbUser_.empty())
        DB::Database::dbUser_ = ConfigManager::Instance.GetGlobal("db_user", "");
    if (DB::Database::dbPass_.empty())
        DB::Database::dbPass_ = ConfigManager::Instance.GetGlobal("db_pass", "");
    if (DB::Database::dbPort_ == 0)
        DB::Database::dbPort_ = static_cast<uint16_t>(ConfigManager::Instance.GetGlobal("db_port", 0));
}

int main(int argc, char* argv[])
{
    ShowLogo();
    if (!ParseCommandline(argc, argv))
    {
        ShowHelp();
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    std::string aux(buff);
#else
    std::string aux(argv[0]);
#endif
    size_t pos = aux.find_last_of("\\/");
    std::string path = aux.substr(0, pos);

    if (gConfigFile.empty())
        gConfigFile = path + "/" + "abdata.lua";

    ConfigManager::Instance.Load(gConfigFile);
    LoadConfig();

    if (gPort == 0)
        gPort = 2770;
    if (gMaxSize == 0)
        gMaxSize = 1024 * 1024 * 1024;

    if (!IO::Logger::logDir_.empty())
        IO::Logger::Close();
    try
    {
        LOG_INFO << "Server config:" << std::endl;
        LOG_INFO << "  Config file: " << gConfigFile << std::endl;
        LOG_INFO << "  Port: " << gPort << std::endl;
        LOG_INFO << "  MaxSize: " << gMaxSize << " bytes" << std::endl;
        LOG_INFO << "  Log dir: " << IO::Logger::logDir_ << std::endl;
        LOG_INFO << "Database drivers:";
        LOG_INFO << " SQLite";   // We always have SQLite
#ifdef USE_MYSQL
        LOG_INFO << " MySQL";
#endif
#ifdef USE_PGSQL
        LOG_INFO << " PostgresSQL";
#endif
#ifdef USE_ODBC
        LOG_INFO << " ODBC";
#endif
        LOG_INFO << std::endl;
        LOG_INFO << "Database config:" << std::endl;
        LOG_INFO << "  Driver: " << DB::Database::driver_ << std::endl;
        LOG_INFO << "  Host: " << DB::Database::dbHost_ << std::endl;
        LOG_INFO << "  Name: " << DB::Database::dbName_ << std::endl;
        LOG_INFO << "  Port: " << DB::Database::dbPort_ << std::endl;
        LOG_INFO << "  User: " << DB::Database::dbUser_ << std::endl;
        LOG_INFO << "  Password: " << (DB::Database::dbPass_.empty() ? "(empty)" : "***********") << std::endl;

        DB::Database* db = DB::Database::Instance();
        if (db == nullptr || !db->IsConnected())
        {
            LOG_ERROR << "Database connection failed" << std::endl;
            return EXIT_FAILURE;
        }

        signal(SIGINT, signal_handler);              // Ctrl+C
        signal(SIGBREAK, signal_handler);            // X clicked
        asio::io_service io_service;
        Server serv(io_service, (uint16_t)gPort, gMaxSize);
        shutdown_handler = [&](int /*signal*/)
        {
            LOG_INFO << "Server shutdown..." << std::endl;
            serv.Shutdown();
        };
        io_service.run();
    }
    catch (std::exception& e)
    {
        LOG_ERROR << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

