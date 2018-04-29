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
#include "Logger.h"
#include "Utils.h"
#include "GameManager.h"
#include <functional>
#include "Random.h"
#include "Connection.h"
#include "SkillManager.h"
#include "Skill.h"
#include "IOEffects.h"
#include "EffectManager.h"
#include "DataProvider.h"
#include "Maintenance.h"
#include "Utils.h"
#include <AB/ProtocolCodes.h>
#include <base64.h>
#include "Profiler.h"

#include "DebugNew.h"

Application* Application::Instance = nullptr;

#ifdef  _WIN32
BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
    assert(Application::Instance);

#ifdef _DEBUG
    LOG_DEBUG << "Got signal " << dwCtrlType << std::endl;
#endif

    switch (dwCtrlType)
    {
    case CTRL_CLOSE_EVENT:                  // Close button or End Task
        Application::Instance->Stop();
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
                Asynch::CreateTask(std::bind(&Application::Stop, Application::Instance))
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
    loaderUniqueLock_(loaderLock_),
    ioService_()
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;
    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
    serviceManager_ = std::make_unique<Net::ServiceManager>(ioService_);
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

    if (!serviceManager_->IsRunning())
        LOG_ERROR << "No services running" << std::endl;

    return serviceManager_->IsRunning();
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

    LOG_INFO << "Connecting to data server...";
    const std::string& dataHost = ConfigManager::Instance[ConfigManager::Key::DataServerHost].GetString();
    uint16_t dataPort = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::DataServerPort].GetInt());
    dataClient_->Connect(dataHost, dataPort);
    if (!dataClient_->IsConnected())
    {
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        exit(EXIT_FAILURE);
    }
    LOG_INFO << "[done]" << std::endl;

    {
        LOG_INFO << "Loading game data...";
        Game::SkillManager::Instance.Load(IO::DataProvider::Instance.GetDataFile("/skills/skills.db"));
        IO::IOEffects::Load(Game::EffectManager::Instance, IO::DataProvider::Instance.GetDataFile("/effects/effects.xml"));
        LOG_INFO << "[done]" << std::endl;
//        std::shared_ptr<Game::Skill> skill = Game::SkillManager::Instance.Get(2);
    }


    // Add Protocols
    uint32_t ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::LoginIP].GetInt());
    uint16_t port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::LoginPort].GetInt());
    if (port != 0)
        serviceManager_->Add<Net::ProtocolLogin>(ip, port);
    ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::AdminIP].GetInt());
    port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::AdminPort].GetInt());
    if (port != 0)
        serviceManager_->Add<Net::ProtocolAdmin>(ip, port);
    ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::StatusIP].GetInt());
    port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::StatusPort].GetInt());
    if (port != 0)
        serviceManager_->Add<Net::ProtocolStatus>(ip, port);
    ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::GameIP].GetInt());
    port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::GamePort].GetInt());
    if (port != 0)
        serviceManager_->Add<Net::ProtocolGame>(ip, port);

    int64_t loadingTime = (Utils::AbTick() - startLoading);

    PrintServerInfo();

    LOG_INFO << "Loading done in ";
    if (loadingTime < 1000)
        LOG_INFO << loadingTime << " ms";
    else
        LOG_INFO << (loadingTime / 1000) << " s";
    LOG_INFO << std::endl;

    Maintenance::Instance.Run();
    Game::GameManager::Instance.Start(serviceManager_.get());

    // Notify we are ready
    loaderSignal_.notify_all();
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server name: " << ConfigManager::Instance[ConfigManager::Key::ServerName].GetString() << std::endl;
    LOG_INFO << "Location: " << ConfigManager::Instance[ConfigManager::Key::Location].GetString() << std::endl;
    LOG_INFO << "Protocol version: " << AB::PROTOCOL_VERSION << std::endl;

    std::list<std::pair<uint32_t, uint16_t>> ports = serviceManager_->GetPorts();
    LOG_INFO << "Listening: ";
    while (ports.size())
    {
        LOG_INFO << Utils::ConvertIPToString(ports.front().first) << ":" << ports.front().second << " ";
        ports.pop_front();
    }
    LOG_INFO << std::endl;

    LOG_INFO << "Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
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
    serviceManager_->Run();
}

void Application::Stop()
{
    LOG_INFO << "Server is shutting down" << std::endl;
    // Before serviceManager_.Stop()
    Net::ConnectionManager::Instance()->CloseAll();
    Maintenance::Instance.Stop();

    serviceManager_->Stop();
}
