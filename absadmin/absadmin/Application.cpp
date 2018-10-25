#include "stdafx.h"
#include "Application.h"
#include "Subsystems.h"
#include "SimpleConfigManager.h"

Application::Application() :
    ServerApp::ServerApp(),
    running_(false)
{
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
}

Application::~Application()
{
    if (running_)
        Stop();
}

bool Application::Initialize(int argc, char** argv)
{
    if (!ServerApp::Initialize(argc, argv))
        return false;

    return true;
}

void Application::Run()
{
}

void Application::Stop()
{
}
