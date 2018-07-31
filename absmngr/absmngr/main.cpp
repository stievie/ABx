// absmngr.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "DataClient.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "process.hpp"
#include "StringUtils.h"
#include "Version.h"
#include <map>
#include <thread>
#include <mutex>
#if _WIN32
#include <windows-kill-library/windows-kill-library.h>
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
#   define CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#endif

using namespace std::chrono_literals;

bool gRunning = false;
std::unique_ptr<IO::DataClient> gClient;
std::vector<std::pair<std::string, std::unique_ptr<TinyProcessLib::Process>>> gProcesses;
std::map<std::string, AB::Entities::Service> gServices;
TinyProcessLib::Process* gDataServerProcess = nullptr;

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
}

static void Run()
{
    gRunning = true;

    std::thread updateThread = std::thread([]()
    {
        while (gRunning)
        {
            std::this_thread::sleep_for(1s);

            std::mutex mutex;
            {
                std::lock_guard<std::mutex> lock(mutex);
                for (const auto& serv : gServices)
                {
                    AB::Entities::Service se = serv.second;
                    if (gClient->Read(se))
                        gServices[se.uuid] = se;
                }
            }
        }
    });

    while (gRunning)
    {
        std::cout << "AB> ";

        std::string input;
        std::getline(std::cin, input);

        if (input.compare("q") == 0 || input.compare("quit") == 0)
        {
            gRunning = false;
            break;
        }
        else if (input.compare("h") == 0 || input.compare("help") == 0)
        {
            std::cout << "q, quit: Quit program, close all managed processes" << std::endl;
            std::cout << "h, help: Show this help" << std::endl;
            std::cout << "proc: List managed processes" << std::endl;
            std::cout << "serv: List services" << std::endl;
        }
        else if (input.compare("proc") == 0)
        {
            int i = 0;
            for (const auto& p : gProcesses)
            {
                auto it = gServices.find(p.first);
                std::cout << i++ << "\t" << p.first;
                if (it != gServices.end())
                {
                    std::cout << "\t" << (*it).second.name;
                }
                std::cout << std::endl;
            }
        }
        else if (input.compare("serv") == 0)
        {
            int i = 0;
            for (const auto& p : gServices)
            {
                std::cout << i++;
                std::cout << "\t" << p.second.name;
                std::cout << "\t" << p.second.location;
                std::cout << "\t" << (p.second.status == AB::Entities::ServiceStatusOnline ? "online" : "offline");
                if (p.second.status == AB::Entities::ServiceStatusOnline)
                    std::cout << "\t" << static_cast<int>(p.second.load) << "%";
                std::cout << std::endl;
            }
        }
        else
        {
            std::vector<std::string> inputParts = Utils::Split(input, " ");
            if (inputParts.size() != 0)
            {
                const std::string& part = inputParts[0];
                if (part.compare("kill") == 0)
                {
                    if (inputParts.size() == 2)
                    {
                    }
                    else
                    {
                        std::cout << "Missing process name" << std::endl;
                    }
                }
            }
        }
    }
    updateThread.join();
}

static void StartDataServer()
{
    AB::Entities::Service s;
    s.name = "abdata";
    std::unique_ptr<TinyProcessLib::Process> process =
        std::make_unique<TinyProcessLib::Process>(L"abdata.exe", L"",
        [](const char*, size_t)
        {
        },
        [](const char*, size_t)
        {
        }
    );
    gDataServerProcess = process.get();
    gProcesses.push_back({ "00000000-0000-0000-0000-000000000000", std::move(process) });
}

static bool ConnectToDataServer()
{
    gClient->Connect("localhost", 2770);
    return gClient->IsConnected();
}

static void StartServer(const AB::Entities::Service& s)
{
    std::wstring cmd(s.file.begin(), s.file.end());
    cmd += L" ";
    std::vector<std::string> args = Utils::Split(s.arguments, " ");
    for (const auto& a : args)
    {
        cmd += L"\"" + std::wstring(a.begin(), a.end()) + L"\" ";
    }
    std::wstring path(s.path.begin(), s.path.end());
    std::unique_ptr<TinyProcessLib::Process> process =
        std::make_unique<TinyProcessLib::Process>(cmd, path,
            [](const char*, size_t) { },
            [](const char*, size_t) { }
    );
    gProcesses.push_back({ s.uuid, std::move(process) });
}

int main()
{
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    ShowLogo();
    std::cout << std::endl;

    std::cout << "Connecting to data server...";

    asio::io_service ioService;
    gClient = std::make_unique<IO::DataClient>(ioService);
    if (!ConnectToDataServer())
    {
        StartDataServer();
        if (!ConnectToDataServer())
        {
            std::cout << std::endl;
            std::cout << "Failed to connect to data server" << std::endl;
            return EXIT_FAILURE;
        }
    }
    std::cout << "[done]" << std::endl;

    AB::Entities::ServiceList sl;
    if (!gClient->Read(sl))
    {
        std::cout << "Error reading service list" << std::endl;
        return EXIT_FAILURE;
    }

    for (const auto& s : sl.uuids)
    {
        AB::Entities::Service serv;
        serv.uuid = s;
        if (!gClient->Read(serv))
        {
            std::cout << "Can not read service " << s << std::endl;
            continue;
        }
        if (gProcesses.size() != 0 && serv.name.compare("abdata") == 0)
        {
            if (gProcesses[0].first.compare("00000000-0000-0000-0000-000000000000") == 0)
                gProcesses[0].first = serv.uuid;
        }
        gServices[serv.uuid] = serv;
        if (serv.status == AB::Entities::ServiceStatusOffline)
        {
            StartServer(serv);
        }
    }

    Run();

#if _WIN32
#endif
    // Kill it in reversed order
    for (auto it = gProcesses.rbegin(); it != gProcesses.rend(); ++it)
    {
        if ((*it).second.get() == gDataServerProcess)
            continue;
        // Can not just terminate it but they handle SIGINT
#if _WIN32
        try
        {
            WindowsKillLibrary::sendSignal((*it).second->GetData().id, WindowsKillLibrary::SIGNAL_TYPE_CTRL_C);
            WaitForSingleObject((*it).second->GetData().handle, 1000);
        }
        catch (const std::invalid_argument& ex)
        {
            std::cout << "Error: " << ex.what() << " PID " << (*it).second->GetData().id << std::endl;
        }
#else
        (*it).second->kill();
        std::this_thread::sleep_for(100ms);
#endif
    }

    if (gDataServerProcess)
    {
        std::this_thread::sleep_for(1000ms);
#if _WIN32
        try
        {

            WindowsKillLibrary::sendSignal(gDataServerProcess->GetData().id, WindowsKillLibrary::SIGNAL_TYPE_CTRL_C);
            WaitForSingleObject(gDataServerProcess->GetData().handle, 1000);
        }
        catch (const std::invalid_argument& ex)
        {
            std::cout << "Error: " << ex.what() << " PID " << gDataServerProcess->GetData().id << std::endl;
        }
#else
        gDataServerProcess->kill();
        std::this_thread::sleep_for(100ms);
#endif
    }

    return EXIT_SUCCESS;
}

#if _WIN32
#pragma comment(lib, "windows-kill-library.lib")
#endif