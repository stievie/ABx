#include "stdafx.h"
#include "Application.h"
#include "Bridge.h"
#include "Acceptor.h"

Application::Application() :
    ServerApp::ServerApp(),
    running_(false),
    ioService_()
{
}

Application::~Application()
{
}

bool Application::Initialize(int argc, char** argv)
{
    if (!ServerApp::Initialize(argc, argv))
        return false;

    return false;
}

void Application::Run()
{
    LOG_INFO << "Server is running" << std::endl;

    // Create one Bridge for each Remote server
    Acceptor acceptor(ioService_, localHost_, localPort_, remoteHost_, remotePort_);
    acceptor.AcceptConnections();

    ioService_.run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown...";
    ioService_.stop();
}
