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
#include "IOEffects.h"
#include "EffectManager.h"
#include "DataProvider.h"
#include "Maintenance.h"
#include "Utils.h"
#include <AB/ProtocolCodes.h>

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

void Application::ParseCommandLine()
{
    for (int i = 0; i != arguments_.size(); i++)
    {
        const std::string& a = arguments_[i];
        if (a.compare("-conf") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                configFile_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -conf" << std::endl;
        }
        else if (a.compare("-log") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                logDir_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -log" << std::endl;
        }
    }
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
    ParseCommandLine();
    if (!logDir_.empty())
    {
        // From the command line
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
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

    if (configFile_.empty())
        configFile_ = path_ + "/" + CONFIG_FILE;
    LOG_INFO << "Loading configuration: " << configFile_ << "...";
    ConfigManager::Instance.Load(configFile_);
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

    {
        LOG_INFO << "Loading game data...";
        IO::IOSkills::Load(Game::SkillManager::Instance, IO::DataProvider::Instance.GetDataFile("/skills/skills.xml"));
        IO::IOEffects::Load(Game::EffectManager::Instance, IO::DataProvider::Instance.GetDataFile("/effects/effects.xml"));
        LOG_INFO << "[done]" << std::endl;
        //std::shared_ptr<Game::Skill> skill = Game::SkillManager::Instance.Get(2);
    }

    // Add Protocols
    uint32_t ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::LoginIP].GetInt());
    uint16_t port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::LoginPort].GetInt());
    if (port != 0)
        serviceManager_.Add<Net::ProtocolLogin>(ip, port);
    ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::AdminIP].GetInt());
    port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::AdminPort].GetInt());
    if (port != 0)
        serviceManager_.Add<Net::ProtocolAdmin>(ip, port);
    ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::StatusIP].GetInt());
    port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::StatusPort].GetInt());
    if (port != 0)
        serviceManager_.Add<Net::ProtocolStatus>(ip, port);
    ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::GameIP].GetInt());
    port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::GamePort].GetInt());
    if (port != 0)
        serviceManager_.Add<Net::ProtocolGame>(ip, port);

    PrintServerInfo();

    LOG_INFO << "Loading done in ";
    int64_t loadingTime = (Utils::AbTick() - startLoading);
    if (loadingTime < 1000)
        LOG_INFO << loadingTime << " ms";
    else
        LOG_INFO << (loadingTime / 1000) << " s";
    LOG_INFO << std::endl;

    Maintenance::Instance.Run();
    Game::GameManager::Instance.Start(&serviceManager_);

    // Notify we are ready
    loaderSignal_.notify_all();
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server name: " << ConfigManager::Instance[ConfigManager::Key::ServerName].GetString() << std::endl;
    LOG_INFO << "Location: " << ConfigManager::Instance[ConfigManager::Key::Location].GetString() << std::endl;
    LOG_INFO << "Protocol version: " << AB::PROTOCOL_VERSION << std::endl;

    std::list<std::pair<uint32_t, uint16_t>> ports = serviceManager_.GetPorts();
    LOG_INFO << "Listening: ";
    while (ports.size())
    {
        LOG_INFO << Utils::ConvertIPToString(ports.front().first) << ":" << ports.front().second << " ";
        ports.pop_front();
    }
    LOG_INFO << std::endl;

    LOG_INFO << "Database drivers:";
#ifdef USE_MYSQL
    LOG_INFO << "\tMySQL";
#endif
#ifdef USE_PGSQL
    LOG_INFO << "\tPostgresSQL";
#endif
    LOG_INFO << std::endl;
}

void Application::Run()
{
    LOG_INFO << "Server is running" << std::endl;
    // If we use a log file close current and reopen as file logger
    if (logDir_.empty())
        logDir_ = ConfigManager::Instance[ConfigManager::Key::LogDir].GetString();
    if (!logDir_.empty() && logDir_.compare(IO::Logger::logDir_) != 0)
    {
        // Different log dir
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }
    serviceManager_.Run();
}

void Application::Stop()
{
    LOG_INFO << "Server is shutting down" << std::endl;
    // Before serviceManager_.Stop()
    Net::ConnectionManager::Instance()->CloseAll();
    Maintenance::Instance.Stop();

    serviceManager_.Stop();
}
