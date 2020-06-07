#include "stdafx.h"
#include "Application.h"
#include "Version.h"
#include <AB/Entities/Service.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/Logo.h>
#include <abscommon/Scheduler.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/Subsystems.h>
#include <abscommon/StringUtils.h>
#include <iostream>

Application::Application() :
    ServerApp(),
    ioService_()
{
    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
}

Application::~Application()
{
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
}

bool Application::LoadMain()
{
    if (configFile_.empty())
    {
        configFile_ = Utils::ConcatPath(path_, "abbots.lua");
    }

    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    if (!config->Load(configFile_))
    {
        std::cerr << "Failed to load config file" << std::endl;
        return false;
    }

    if (Utils::Uuid::IsEmpty(serverId_))
        serverId_ = config->GetGlobalString("server_id", Utils::Uuid::EMPTY_UUID);
    if (serverName_.empty())
        serverName_ = config->GetGlobalString("server_name", "abbots");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobalString("location", "--");
    if (logDir_.empty())
        logDir_ = config->GetGlobalString("log_dir", "");

    return true;
}

void Application::ShowVersion()
{
    std::cout << SERVER_PRODUCT_NAME << " " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR << std::endl;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
}

void Application::ShowLogo()
{
    std::cout << "This is " << SERVER_PRODUCT_NAME << std::endl;
    std::cout << "Version " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
    std::cout << "(C) 2017-" << SERVER_YEAR << std::endl;
    std::cout << std::endl;

    std::cout << AB_CONSOLE_LOGO << std::endl;

    std::cout << std::endl;
}

void Application::Update()
{
    if (running_)
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(16, std::bind(&Application::Update, this)));
}

void Application::MainLoop()
{
    // Main thread
    while (running_)
    {
        ioService_.run();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
}

bool Application::Initialize(const std::vector<std::string>& args)
{
    if (!ServerApp::Initialize(args))
        return false;

    if (!ParseCommandLine())
        return false;

    if (!sa::arg_parser::get_value<bool>(parsedArgs_, "nologo", false))
        ShowLogo();

    if (!LoadMain())
        return false;

    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();

    return true;
}

void Application::Run()
{
    GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(16, std::bind(&Application::Update, this)));

    running_ = true;
    std::cout << "Running" << std::endl;
    MainLoop();
}

void Application::Stop()
{
    running_ = false;
    std::cout << "Stopped" << std::endl;
}
