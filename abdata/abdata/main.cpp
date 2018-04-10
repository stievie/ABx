// abdata.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include "Server.h"
#include "Logger.h"
#include "Database.h"

int gPort = 2770;
int gMaxSize = 1024 * 1024 * 1024;

static void ShowLogo()
{
    std::cout << "abdata" << std::endl;
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
        else if (arg.compare("-dbfile") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                DB::Database::dbFile_ = std::string(argv[i]);
            }
            else
                LOG_WARNING << "Missing argument for -dbfile" << std::endl;
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

int main(int argc, char* argv[])
{
    ShowLogo();
    if (!ParseCommandline(argc, argv))
    {
        ShowHelp();
        return 1;
    }

    if (!IO::Logger::logDir_.empty())
        IO::Logger::Close();
    try
    {
        std::cout << "Port: " << gPort << std::endl;
        std::cout << "MaxSize: " << gMaxSize << " bytes" << std::endl;
        std::cout << "DB driver: " << DB::Database::driver_ << std::endl;
        std::cout << "DB file (SQlite): " << DB::Database::dbFile_ << std::endl;
        std::cout << "DB host: " << DB::Database::dbHost_ << std::endl;
        std::cout << "DB name: " << DB::Database::dbName_ << std::endl;
        std::cout << "DB port: " << DB::Database::dbPort_ << std::endl;

        asio::io_service io_service;
        Server serv(io_service, (uint16_t)gPort, gMaxSize);
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

