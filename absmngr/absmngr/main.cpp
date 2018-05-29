// absmngr.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "DataClient.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "process.hpp"
#include "StringUtils.h"

bool gRunning = false;
std::unique_ptr<IO::DataClient> gClient;
std::vector<std::pair<std::string, std::unique_ptr<TinyProcessLib::Process>>> gProcesses;

static void ShowLogo()
{
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
        else if (input.compare("list") == 0)
        {
            int i = 0;
            for (const auto& p : gProcesses)
            {
                std::cout << i++ << "\t" << p.first << std::endl;
            }
        }
    }
}

static void StartDataServer()
{
    std::unique_ptr<TinyProcessLib::Process> process =
        std::make_unique<TinyProcessLib::Process>(L"abdata.exe", L"",
        [](const char*, size_t)
        {
        },
        [](const char*, size_t)
        {
        }
    );
    gProcesses.push_back({ "abdata", std::move(process) });
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
    gProcesses.push_back({ s.name, std::move(process) });
}

int main()
{
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
        if (serv.status == AB::Entities::ServiceStatusOffline)
        {
            StartServer(serv);
        }
    }

    Run();

    // Kill it in reversed order
    for (auto it = gProcesses.rbegin(); it != gProcesses.rend(); ++it)
    {
        (*it).second->kill();
    }
    return EXIT_SUCCESS;
}

