#include "stdafx.h"
#include "AppService.h"

#if defined(_WIN32) && defined (WIN_SERVICE)
#include "ThreadPool.h"
#include "Subsystems.h"
#include "Logger.h"

AppService::AppService() :
    System::WinServiceBase(SERVICE_NAME)
{
}

void AppService::OnStart(DWORD dwArgc, PWSTR* pszArgv)
{
    app_ = std::make_shared<Application>();
    if (!app_->InitializeW(dwArgc, pszArgv))
    {
        WriteEventLogEntry(L"Application initialization failed", EVENTLOG_ERROR_TYPE);
        app_.reset();
        return;
    }
    thread_ = std::thread(&Application::Run, app_);
}

void AppService::OnStop()
{
    if (app_)
    {
        app_->Stop();
        thread_.join();
    }
}

void AppService::OnShutdown()
{
    if (app_)
    {
        app_->Stop();
        thread_.join();
    }
}

#endif
