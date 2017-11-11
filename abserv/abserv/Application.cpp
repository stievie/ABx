#include "stdafx.h"
#include "Application.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include "ProtocolGame.h"
#include "ProtocolLogin.h"
#include "ProtocolAdmin.h"
#include "ProtocolStatus.h"
#include "ConfigManager.h"
#include "Task.h"
#include <thread>
#include <chrono>
#include "Logger.h"
#include "Utils.h"
#include "GameManager.h"
#include "Rsa.h"
#include <functional>
#include "Random.h"
#include <ostream>
#include <iostream>
#include "Connection.h"
#include "Database.h"
#include "DHKeys.h"
#include "Aes.h"
#include "SkillManager.h"
#include "Skill.h"
#include "IOSkills.h"

#include "DebugNew.h"

Application* gApplication = nullptr;

#ifdef  _WIN32
BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
    assert(gApplication);

#ifdef _DEBUG
    LOG_DEBUG << "Got signal " << dwCtrlType << std::endl;
#endif

    switch (dwCtrlType)
    {
    case CTRL_CLOSE_EVENT:                  // Close button or End Task
        gApplication->Stop();
        return TRUE;
    case CTRL_C_EVENT:                      // Ctrl+C
    {
        // Either it stops or it crashes...
        std::cout << "Stopping server? [y/n]: ";
        std::string answer;
        std::cin >> answer;
        if (answer.compare("y") == 0)
        {
            Asynch::Dispatcher::Instance.Add(
                Asynch::CreateTask(std::bind(&Application::Stop, gApplication))
            );
        }
        return TRUE;
    }
    default:
        return FALSE;
    }
}
#endif

Application::Application() :
    loaderUniqueLock_(loaderLock_)
{
    assert(gApplication == nullptr);
    gApplication = this;
}

Application::~Application()
{
    Game::GameManager::Instance.Stop();
    Asynch::Scheduler::Instance.Stop();
    Asynch::Dispatcher::Instance.Stop();
}

bool Application::Initialize(int argc, char** argv)
{
    using namespace std::chrono_literals;
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandlerRoutine, TRUE);
#endif
#ifdef _WIN32
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    std::string aux(buff);
#else
    std::string aux(argv[0]);
#endif
    size_t pos = aux.find_last_of("\\/");
    path_ = aux.substr(0, pos);
    for (int i = 0; i < argc; i++)
    {
        arguments_.push_back(std::string(argv[i]));
    }

    Asynch::Dispatcher::Instance.Start();
    Asynch::Scheduler::Instance.Start();

    Asynch::Dispatcher::Instance.Add(Asynch::CreateTask(std::bind(&Application::MainLoader, this)));
    loaderSignal_.wait(loaderUniqueLock_);
    std::this_thread::sleep_for(100ms);

    if (!serviceManager_.IsRunning())
        LOG_ERROR << "No services running" << std::endl;

    return serviceManager_.IsRunning();
}

void Application::MainLoader()
{
    int64_t startLoading = Utils::AbTick();

    LOG_INFO << "Loading..." << std::endl;

    LOG_INFO << "Loading configuration...";
    ConfigManager::Instance.Load(path_ + "/" + CONFIG_FILE);
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Initializing RNG...";
    Utils::Random::Instance.Initialize();
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Loading cryptographic keys...";
    {
        std::string keyFile = ConfigManager::Instance[ConfigManager::CryptoKeys].GetString();
        if (keyFile.empty())
            keyFile  = path_ + "/" + DH_KEYS_FILE;
        if (!Crypto::DHKeys::Instance.LoadKeys(keyFile))
        {
            Crypto::DHKeys::Instance.GenerateKeys();
            Crypto::DHKeys::Instance.SaveKeys(keyFile);
        }
    }
    LOG_INFO << "[done]" << std::endl;
    LOG_INFO << "Testing AES...";
    if (Crypto::Aes::SelfTest())
        LOG_INFO << "[success]" << std::endl;
    else
        LOG_INFO << "[FAILED]" << std::endl;

    // DB ----------------
    LOG_INFO << "Creating DB connection...";
    DB::Database* db = DB::Database::Instance();
    if (db == nullptr || !db->IsConnected())
    {
        LOG_ERROR << "Database connection failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Checking DB schema version...";
    // TODO:
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Loading skills...";
    if (IO::IOSkills::Load(Game::SkillManager::Instance, ConfigManager::Instance.GetDataFile("/skills/skills.xml")))
        LOG_INFO << "[done]" << std::endl;
    else
        LOG_INFO << "[failed]" << std::endl;
//    std::shared_ptr<Game::Skill> skill = Game::SkillManager::Instance.GetSkill(1);

    serviceManager_.Add<Net::ProtocolLogin>(static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::LoginPort].GetInt()));
    serviceManager_.Add<Net::ProtocolAdmin>(static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::AdminPort].GetInt()));
    serviceManager_.Add<Net::ProtocolStatus>(static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::StatusPort].GetInt()));
    serviceManager_.Add<Net::ProtocolGame>(static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::GamePort].GetInt()));

    PrintServerInfo();

    LOG_INFO << "Loading done in ";
    int64_t loadingTime = (Utils::AbTick() - startLoading);
    if (loadingTime < 1000)
        LOG_INFO << loadingTime << " ms";
    else
        LOG_INFO << (loadingTime / 1000) << " s";
    LOG_INFO << std::endl;

    Game::GameManager::Instance.Start(&serviceManager_);

    // Notify we are ready
    loaderSignal_.notify_all();
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server name: " << ConfigManager::Instance[ConfigManager::Key::ServerName].GetString() << std::endl;
    LOG_INFO << "Location: " << ConfigManager::Instance[ConfigManager::Key::Location].GetString() << std::endl;

    std::list<uint16_t> ports = serviceManager_.GetPorts();
    LOG_INFO << "Local ports: ";
    while (ports.size())
    {
        LOG_INFO << ports.front() << "\t";
        ports.pop_front();
    }
    LOG_INFO << std::endl;
}

void Application::Run()
{
    LOG_INFO << "Server is running" << std::endl;
    // If we use a log file close current and reopen as file logger
    std::string logDir = ConfigManager::Instance[ConfigManager::Key::LogDir].GetString();
    if (!logDir.empty())
    {
        LOG_INFO << "Log directory: " << logDir << std::endl;
        IO::Logger::logDir_ = logDir;
        IO::Logger::Close();
    }
    serviceManager_.Run();
}

void Application::Stop()
{
    LOG_INFO << "Server is shutting down" << std::endl;
    // Before serviceManager_.Stop()
    Net::ConnectionManager::GetInstance()->CloseAll();

    serviceManager_.Stop();
}
