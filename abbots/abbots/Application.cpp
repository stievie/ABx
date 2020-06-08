/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "Application.h"
#include "BotClient.h"
#include "Version.h"
#include <sa/Compiler.h>
#include <AB/Entities/Service.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/Logger.h>
#include <abscommon/Logo.h>
#include <abscommon/Scheduler.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/Subsystems.h>
#include <abscommon/StringUtils.h>
#include <iostream>

Application::Application() :
    ServerApp(),
    ioService_(std::make_shared<asio::io_service>())
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

    LOG_INFO << "Loading configuration...";
    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    if (!config->Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

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

void Application::CreateBots()
{
    // TODO: Create all the bots.
    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    int index = 1;
    accounts_.push_back({});
    while (config->GetTable("account" + std::to_string(index),
        [this](const std::string& name, const Utils::Variant& value) -> Iteration
    {
        auto& current = accounts_.back();
        if (name.compare("name") == 0)
        {
            current.name = value.GetString();
        }
        else if (name.compare("pass") == 0)
        {
            current.pass = value.GetString();
        }
        if (name.compare("char") == 0)
        {
            current.character = value.GetString();
        }
        return Iteration::Continue;
    }))
    {
        accounts_.push_back({});
        ++index;
    }
    // The last is empty
    accounts_.pop_back();

    for (const auto& account : accounts_)
    {
        auto client = std::make_unique<BotClient>(ioService_);
        client->username_ = account.name;
        client->password_ = account.pass;
        client->characterName_ = account.character;
        clients_.push_back(std::move(client));
    }
}

void Application::StartBots()
{
    for (const auto& client : clients_)
    {
        LOG_INFO << "Starting Bot " << client->username_ << " with character " << client->characterName_ << std::endl;
//        client->Login();
    }
}

void Application::Shutdown()
{
    for (const auto& client : clients_)
    {
        client->Logout();
    }
}

void Application::Update()
{
    uint32_t time;
    if (lastUpdate_ == 0)
        time = 16;
    else
        time = Utils::TimeElapsed(lastUpdate_);

    int64_t startTick = Utils::Tick();
    for (const auto& client : clients_)
    {
        client->Update(time);
    }
    uint32_t timeSpent = Utils::TimeElapsed(startTick);

    lastUpdate_ = Utils::Tick();
    uint32_t nextSchedule;
    if (timeSpent < 16)
        nextSchedule = 16 - timeSpent;
    else
        nextSchedule = 1;

    if (running_)
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(nextSchedule, std::bind(&Application::Update, this)));
}

void Application::MainLoop()
{
    // Main thread
    while (running_)
    {
        ioService_->run();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(16ms);
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
    CreateBots();

    return true;
}

void Application::Run()
{
    GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(16, std::bind(&Application::Update, this)));

    running_ = true;
    StartBots();
    LOG_INFO << "Bot Army is running" << std::endl;
    MainLoop();
}

void Application::Stop()
{
    running_ = false;
    LOG_INFO << "Bot Army stopped" << std::endl;
}
