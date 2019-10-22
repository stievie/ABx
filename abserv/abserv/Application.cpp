#include "stdafx.h"
#include "Application.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include "ProtocolGame.h"
#include "ConfigManager.h"
#include "Task.h"
#include "Logger.h"
#include "StringUtils.h"
#include "GameManager.h"
#include "Random.h"
#include "Connection.h"
#include "SkillManager.h"
#include "Skill.h"
#include "EffectManager.h"
#include "DataProvider.h"
#include "Maintenance.h"
#include <AB/ProtocolCodes.h>
#include "PlayerManager.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "Connection.h"
#include "BanManager.h"
#include "CpuUsage.h"
#include "ThreadPool.h"
#include "EffectManager.h"
#include "DataClient.h"
#include <AB/DHKeys.hpp>
#include "AiRegistry.h"
#include "AiLoader.h"
#include "PartyManager.h"
#include "ItemFactory.h"
#include <sa/ConditionSleep.h>
#include "ItemsCache.h"
#include "UuidUtils.h"
#include "GuildManager.h"
#include "OutputMessage.h"
#include "NetworkMessage.h"

Application* Application::Instance = nullptr;

Application::Application() :
    ServerApp::ServerApp(),
    ioService_()
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;

    serverType_ = AB::Entities::ServiceTypeGameServer;

    static constexpr size_t EXPECTED_CONNECTIONS = 4096;
    static constexpr size_t NETWORKMESSAGE_POOLCOUNT = EXPECTED_CONNECTIONS * 2;
    static constexpr size_t NETWORKMESSAGE_POOLSIZE = Net::NETWORKMESSAGE_MAXSIZE * NETWORKMESSAGE_POOLCOUNT;
    static constexpr size_t OUTPUTMESSAGE_POOLCOUNT = EXPECTED_CONNECTIONS * 2;
    static constexpr size_t OUTPUTMESSAGE_POOLSIZE = Net::OUTPUTMESSAGE_SIZE * OUTPUTMESSAGE_POOLCOUNT;

    Subsystems::Instance.CreateSubsystem<Net::NetworkMessage::MessagePool>(NETWORKMESSAGE_POOLSIZE);
    Subsystems::Instance.CreateSubsystem<Net::PoolWrapper::MessagePool>(OUTPUTMESSAGE_POOLSIZE);
    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<Asynch::ThreadPool>();
    Subsystems::Instance.CreateSubsystem<Net::ConnectionManager>();
    Subsystems::Instance.CreateSubsystem<IO::DataClient>(ioService_);
    Subsystems::Instance.CreateSubsystem<Net::MessageClient>(ioService_);

    Subsystems::Instance.CreateSubsystem<Crypto::Random>();
    Subsystems::Instance.CreateSubsystem<Crypto::DHKeys>();
    Subsystems::Instance.CreateSubsystem<ConfigManager>();
    Subsystems::Instance.CreateSubsystem<IO::DataProvider>();

    Subsystems::Instance.CreateSubsystem<Auth::BanManager>();
    Subsystems::Instance.CreateSubsystem<Game::GameManager>();
    Subsystems::Instance.CreateSubsystem<Game::PlayerManager>();
    Subsystems::Instance.CreateSubsystem<Game::PartyManager>();
    Subsystems::Instance.CreateSubsystem<Game::GuildManager>();

    Subsystems::Instance.CreateSubsystem<Game::EffectManager>();
    Subsystems::Instance.CreateSubsystem<Game::Chat>();
    Subsystems::Instance.CreateSubsystem<Game::SkillManager>();
    Subsystems::Instance.CreateSubsystem<Game::ItemFactory>();
    Subsystems::Instance.CreateSubsystem<Game::ItemsCache>();
    Subsystems::Instance.CreateSubsystem<AI::AiRegistry>();
    auto* reg = GetSubsystem<AI::AiRegistry>();
    Subsystems::Instance.CreateSubsystem<AI::AiLoader>(*reg);

    serviceManager_ = std::make_unique<Net::ServiceManager>(ioService_);

    cli_.push_back({ "autoterm", { "-autoterm", "--auto-terminate" }, "Automatic stop application", false, false, sa::arg_parser::option_type::none });
    cli_.push_back({ "temp", { "-temp", "--temporary" }, "Temporary application", false, false, sa::arg_parser::option_type::none });
    cli_.push_back({ "aiserver", { "-aiserver", "--ai-server" }, "Start AI server", false, false, sa::arg_parser::option_type::none });
}

Application::~Application()
{
    serviceManager_->Stop();
    GetSubsystem<Net::ConnectionManager>()->CloseAll();
    GetSubsystem<Asynch::ThreadPool>()->Stop();
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
}

bool Application::ParseCommandLine()
{
    if (!ServerApp::ParseCommandLine())
        return false;

    if (sa::arg_parser::get_value<bool>(parsedArgs_, "autoterm", false))
    {
        // Must be set with command line argument. Can not be set with the config file.
        autoTerminate_ = true;
        // Implies temporary
        temporary_ = true;
    }
    if (sa::arg_parser::get_value<bool>(parsedArgs_, "temp", false))
        temporary_ = true;
    if (sa::arg_parser::get_value<bool>(parsedArgs_, "aiserver", false))
        aiServer_ = true;
    return true;
}

void Application::ShowHelp()
{
    std::cout << sa::arg_parser::get_help("abserv", cli_);
}

bool Application::Initialize(const std::vector<std::string>& args)
{
    if (!ServerApp::Initialize(args))
        return false;

    if (!ParseCommandLine())
    {
        std::cout << cmdErrors_;
        ShowHelp();
        return false;
    }

    if (!logDir_.empty())
    {
        // From the command line
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();
    GetSubsystem<Asynch::ThreadPool>()->Start();

    if (!LoadMain())
        return false;

#if defined(SCENE_VIEWER)
    sceneViewer_ = std::make_shared<Debug::SceneViewer>();
    if (!sceneViewer_->Initialize())
    {
        return false;
    }
#endif

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    if (!serviceManager_->IsRunning())
        LOG_ERROR << "No services running" << std::endl;

    return serviceManager_->IsRunning();
}

void Application::SpawnServer()
{
    Spawn("-autoterm");
}

void Application::HandleCreateInstanceMessage(const Net::MessageMsg& msg)
{
    IO::PropReadStream createInstStream;
    if (!msg.GetPropStream(createInstStream))
    {
        return;
    }
    std::string hostingServer;
    if (!createInstStream.ReadString(hostingServer))
        return;
    // Not we should host the game? Shouldn't happen this message is only sent to us
    if (!Utils::Uuid::IsEqual(hostingServer, GetServerId()))
        return;
    std::string mapUuid;
    if (!createInstStream.ReadString(mapUuid))
        return;
    std::string instanceUuid;
    if (!createInstStream.ReadString(instanceUuid))
        return;
    auto* gameMan = GetSubsystem<Game::GameManager>();
    auto inst = gameMan->GetOrCreateInstance(mapUuid, instanceUuid);
    if (!inst)
    {
        LOG_ERROR << "Unable to create instance. Map UUID " << mapUuid << ", instance UUID " << instanceUuid << std::endl;
    }
}

void Application::HandleMessage(const Net::MessageMsg& msg)
{
    switch (msg.type_)
    {
    case Net::MessageType::Shutdown:
    {
        std::string serverId = msg.GetBodyString();
        if (Utils::Uuid::IsEqual(serverId, serverId_))
            // Can't use Dispatcher because Stop() must run in a different thread. Stop() will wait
            // until all games are deleted, and games are deleted in the Dispatcher thread.
            GetSubsystem<Asynch::ThreadPool>()->Enqueue(&Application::Stop, this);
        break;
    }
    case Net::MessageType::Spawn:
    {
        GetSubsystem<Asynch::ThreadPool>()->Enqueue(&Application::SpawnServer, this);
        break;
    }
    case Net::MessageType::ClearCache:
    {
        std::string serverId = msg.GetBodyString();
        if (Utils::Uuid::IsEqual(serverId, serverId_))
        {
            auto* dataProvider = GetSubsystem<IO::DataProvider>();
            GetSubsystem<Asynch::Dispatcher>()->Add(Asynch::CreateTask(std::bind(&IO::DataProvider::ClearCache, dataProvider)));
        }
        break;
    }
    case Net::MessageType::ServerJoined:
    case Net::MessageType::ServerLeft:
    {
        IO::PropReadStream prop;
        if (msg.GetPropStream(prop))
        {
            AB::Entities::Service s;
            prop.Read<AB::Entities::ServiceType>(s.type);
            prop.ReadString(s.uuid);

            if ((!Utils::Uuid::IsEqual(s.uuid, serverId_)) && (s.type == AB::Entities::ServiceTypeGameServer))
            {
                // Notify players another game server joined/left. Wait some time until the
                // service list is updated.
                GetSubsystem<Asynch::Scheduler>()->Add(
                    Asynch::CreateScheduledTask(500, [this, msg]()
                {
                    msgDispatcher_->Dispatch(msg);
                })
                );
            }
        }
        break;
    }
    case Net::MessageType::CreateGameInstance:
        HandleCreateInstanceMessage(msg);
        break;
    default:
        // All other message are delivered by the message dispatcher
        msgDispatcher_->Dispatch(msg);
        break;
    }
}

bool Application::LoadMain()
{
    int64_t startLoading = Utils::Tick();

    LOG_INFO << "Loading..." << std::endl;

    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = path_ + "/" + "abserv_svc.lua";
#else
        configFile_ = path_ + "/" + CONFIG_FILE;
#endif
    }
    LOG_INFO << "Loading configuration: " << configFile_ << "...";
    auto* config = GetSubsystem<ConfigManager>();
    if (!config)
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to get config manager" << std::endl;
        return false;
    }
    if (!config->Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to load configuration file" << std::endl;
        return false;
    }
    if (Utils::Uuid::IsEmpty(serverId_))
        serverId_ = (*config)[ConfigManager::Key::ServerID].GetString();
    if (machine_.empty())
        machine_ = (*config)[ConfigManager::Key::Machine].GetString();

    if (serverName_.empty())
        serverName_ = (*config)[ConfigManager::Key::ServerName].GetString();
    if (serverLocation_.empty())
        serverLocation_ = (*config)[ConfigManager::Key::Location].GetString();
    Net::ProtocolGame::serverId_ = GetServerId();

    Net::ConnectionManager::maxPacketsPerSec = static_cast<uint32_t>((*config)[ConfigManager::Key::MaxPacketsPerSecond].GetInt64());
    Auth::BanManager::LoginTries = static_cast<uint32_t>((*config)[ConfigManager::Key::LoginTries].GetInt64());
    Auth::BanManager::LoginRetryTimeout = static_cast<uint32_t>((*config)[ConfigManager::Key::LoginRetryTimeout].GetInt64());

    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Initializing RNG...";
    auto* rnd = GetSubsystem<Crypto::Random>();
    rnd->Initialize();
    ai::randomSeed(rnd->Get<uint32_t>());
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Loading encryption keys...";
    auto* keys = GetSubsystem<Crypto::DHKeys>();
    if (!keys)
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to get encryption keys" << std::endl;
        return false;
    }
    if (!keys->LoadKeys(GetKeysFile()))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to load encryption keys from " << GetKeysFile() << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    // AI
    LOG_INFO << "Loading behavior trees...";
    auto* aiReg = GetSubsystem<AI::AiRegistry>();
    aiReg->Initialize();
    const std::string& btFile = (*config)[ConfigManager::Key::Behaviours].GetString();
    auto* dp = GetSubsystem<IO::DataProvider>();
    std::string absBtFile = dp->GetDataFile(btFile);
    auto aiLoader = GetSubsystem<AI::AiLoader>();
    if (!aiLoader->Init(absBtFile))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }
    // AI server
    if (!aiServer_)
        aiServer_ = (*config)[ConfigManager::Key::AIServer].GetBool();
    if (aiServer_)
    {
        aiServerIp_ = (*config)[ConfigManager::Key::AIServerIp].GetString();
        aiServerPort_ = static_cast<uint16_t>((*config)[ConfigManager::Key::AIServerPort].GetInt());
        ai::Server* server = new ai::Server(*aiReg, static_cast<short>(aiServerPort_), aiServerIp_);
        Subsystems::Instance.RegisterSubsystem(server);
    }

    LOG_INFO << "[done]" << std::endl;

    // Data server
    LOG_INFO << "Connecting to data server...";
    const std::string& dataHost = (*config)[ConfigManager::Key::DataServerHost].GetString();
    uint16_t dataPort = static_cast<uint16_t>((*config)[ConfigManager::Key::DataServerPort].GetInt());
    auto* dataClient = GetSubsystem<IO::DataClient>();
    dataClient->Connect(dataHost, dataPort);
    if (!dataClient->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;
    if (serverName_.empty() || serverName_.compare("generic") == 0)
    {
        serverName_ = GetFreeName(dataClient);
    }

    LOG_INFO << "Connecting to message server...";
    const std::string& msgHost = (*config)[ConfigManager::Key::MessageServerHost].GetString();
    uint16_t msgPort = static_cast<uint16_t>((*config)[ConfigManager::Key::MessageServerPort].GetInt());

    auto* msgClient = GetSubsystem<Net::MessageClient>();
    msgClient->Connect(msgHost, msgPort, std::bind(&Application::HandleMessage, this, std::placeholders::_1));
    msgDispatcher_ = std::make_unique<MessageDispatcher>();
    if (msgClient->IsConnected())
        LOG_INFO << "[done]" << std::endl;
    else
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to message server" << std::endl;
    }

    LOG_INFO << "Loading Game items...";
    auto* itemFactory = GetSubsystem<Game::ItemFactory>();
    itemFactory->Initialize();
    LOG_INFO << "[done]" << std::endl;

    if (serverHost_.empty())
        serverHost_ = (*config)[ConfigManager::Key::GameHost].GetString();
    uint32_t ip;
    if (!serverIp_.empty())
        ip = Utils::ConvertStringToIP(serverIp_);
    else
        ip = static_cast<uint32_t>((*config)[ConfigManager::Key::GameIP].GetInt());
    if (serverPort_ == std::numeric_limits<uint16_t>::max())
        serverPort_ = static_cast<uint16_t>((*config)[ConfigManager::Key::GamePort].GetInt());
    else if (serverPort_ == 0)
        serverPort_ = Net::ServiceManager::GetFreePort();
    if (serverPort_ != 0)
    {
        if (!serviceManager_->Add<Net::ProtocolGame>(ip, serverPort_, [](uint32_t remoteIp) -> bool
        {
            return GetSubsystem<Auth::BanManager>()->AcceptConnection(remoteIp);
        }))
            return false;
    }
    else
    {
        LOG_ERROR << "Port can not be 0" << std::endl;
    }


    uint32_t loadingTime = Utils::TimeElapsed(startLoading);

    PrintServerInfo();

    LOG_INFO << "Loading done in ";
    if (loadingTime < 1000)
        LOG_INFO << loadingTime << " ms";
    else
        LOG_INFO << (loadingTime / 1000) << " s";
    LOG_INFO << std::endl;

    maintenance_.Run();
    GetSubsystem<Game::GameManager>()->Start();

    return true;
}

void Application::PrintServerInfo()
{
    auto* config = GetSubsystem<ConfigManager>();
    auto* dataClient = GetSubsystem<IO::DataClient>();
    auto* msgClient = GetSubsystem<Net::MessageClient>();
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Machine: " << machine_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Protocol version: " << AB::PROTOCOL_VERSION << std::endl;

    std::list<std::pair<uint32_t, uint16_t>> ports = serviceManager_->GetPorts();
    LOG_INFO << "  Listening: ";
    while (ports.size())
    {
        LOG_INFO << Utils::ConvertIPToString(ports.front().first) << ":" << ports.front().second << " ";
        ports.pop_front();
    }
    LOG_INFO << std::endl;
    if (aiServer_)
        LOG_INFO << "  AI Server listening: " << aiServerIp_ << ":" << aiServerPort_ << std::endl;
    else
        LOG_INFO << "  AI Server: (not running)" << std::endl;
    LOG_INFO << "  Auto terminate: " << autoTerminate_ << std::endl;
    LOG_INFO << "  Temporary: " << temporary_ << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Recording games: " << (*config)[ConfigManager::Key::RecordGames].GetBool() << std::endl;
    const std::string& recDir = (*config)[ConfigManager::Key::RecordingsDir].GetString();
    LOG_INFO << "  Recording directory: " << (recDir.empty() ? "(empty)" : recDir) << std::endl;
    LOG_INFO << "  Background threads: " << GetSubsystem<Asynch::ThreadPool>()->GetNumThreads() << std::endl;

    LOG_INFO << "  Data Server: " << dataClient->GetHost() << ":" << dataClient->GetPort() << std::endl;
    LOG_INFO << "  Message Server: " << msgClient->GetHost() << ":" << msgClient->GetPort() << std::endl;
}

void Application::Run()
{
    auto* config = GetSubsystem<ConfigManager>();
    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    if (!dataClient->Read(serv))
    {
        if (!temporary_)
        {
            // Temporary services do not exist in DB
            LOG_WARNING << "Unable to read service with UUID " << serv.uuid << std::endl;
        }
    }
    UpdateService(serv);
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.temporary = temporary_;
    serv.startTime = Utils::Tick();
    serv.heartbeat = Utils::Tick();
    dataClient->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient->Invalidate(sl);

#if defined(SCENE_VIEWER)
    sceneViewer_->Run();
#endif

    LOG_INFO << "Server is running" << std::endl;
    // If we use a log file close current and reopen as file logger
    if (logDir_.empty())
        logDir_ = (*config)[ConfigManager::Key::LogDir].GetString();
    if (!logDir_.empty() && !Utils::SameFilename(logDir_, IO::Logger::logDir_))
    {
        // Different log dir
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    SendServerJoined(GetSubsystem<Net::MessageClient>(), serv);

    running_ = true;
    if (aiServer_)
    {
        ai::Server* aiServ = GetSubsystem<ai::Server>();
        aiServ->start();
    }
    serviceManager_->Run();
    ioService_.run();
}

void Application::Stop()
{
    if (!running_)
        return;

    // Stop can not be called from the Dispatcher thread.
    // TODO: I think this should be an assert()
    if (GetSubsystem<Asynch::Dispatcher>()->IsDispatcherThread())
    {
        LOG_WARNING << "Application::Stop() was called from the Dispatcher thread" << std::endl;
        GetSubsystem<Asynch::ThreadPool>()->Enqueue(&Application::Stop, this);
        return;
    }

    // On Windows this can't take too long, or Windows will just terminate the process
    // when stopped with a signal.

#if defined(SCENE_VIEWER)
    sceneViewer_->Stop();
#endif

    auto* dataClient = GetSubsystem<IO::DataClient>();
    running_ = false;
    LOG_INFO << "Server shutdown..." << std::endl;

    AB::Entities::Service serv;
    serv.uuid = GetServerId();

    auto* msgClient = GetSubsystem<Net::MessageClient>();

    if (dataClient->Read(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = Utils::Tick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;

        SendServerLeft(msgClient, serv);

        if (!temporary_)
            dataClient->Update(serv);
        else
            // If autoterm -> temporary -> dynamically spawned -> delete from DB
            dataClient->Delete(serv);

        AB::Entities::ServiceList sl;
        dataClient->Invalidate(sl);
    }
    else
        LOG_ERROR << "Unable to read service" << std::endl;

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(50ms);

    GetSubsystem<Game::PlayerManager>()->KickAllPlayers();
    auto* gameMan = GetSubsystem<Game::GameManager>();
    gameMan->Stop();

    // We must wait until all games stopped
    sa::ConditionSleep([&]() {
        return gameMan->GetGameCount() == 0;
    }, 2000);
    std::this_thread::sleep_for(50ms);

    // Before serviceManager_.Stop()
    maintenance_.Stop();

    msgClient->Close();
    ioService_.stop();
}

std::string Application::GetKeysFile() const
{
    const std::string& keys = (*GetSubsystem<ConfigManager>())[ConfigManager::ServerKeys].GetString();
    if (!keys.empty())
        return keys;
    return Utils::AddSlash(path_) + "abserver.dh";
}

unsigned Application::GetLoad()
{
    static System::CpuUsage usage;
    if (Utils::TimeElapsed(lastLoadCalc_) > 1000 || loads_.empty())
    {
        lastLoadCalc_ = Utils::Tick();
        size_t playerCount = GetSubsystem<Game::PlayerManager>()->GetPlayerCount();
        float ld = (static_cast<float>(playerCount) / static_cast<float>(SERVER_MAX_CONNECTIONS)) * 100.0f;
        unsigned load = static_cast<unsigned>(ld);

        unsigned utilization = GetSubsystem<Asynch::Dispatcher>()->GetUtilization();
        if (utilization > load)
            load = utilization;

        unsigned l = static_cast<unsigned>(usage.GetUsage());
        if (l > load)
            // Use the higher value
            load = l;

        {
            // Get memory pool usage
            std::lock_guard<std::mutex> lock(lock_);
            const auto ompi = Net::OutputMessagePool::GetPoolInfo();
            if (ompi.usage > load)
                load = l;

            const auto nwpi = Net::NetworkMessage::GetPoolInfo();
            if (nwpi.usage > load)
                load = l;
        }

        if (load > 100)
            load = 100;

        while (loads_.size() > 9)
            loads_.erase(loads_.begin());
        loads_.push_back(load);
    }
    return GetAvgLoad();
}
