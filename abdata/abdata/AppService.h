#pragma once

#if defined(_WIN32) && defined(WIN_SERVICE)
#include "WinServiceBase.h"
#include "Application.h"

// Internal name of the service
#define SERVICE_NAME             L"ABDataService"
// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"AB Data Service"
// Service start options.
#define SERVICE_START_TYPE       SERVICE_DEMAND_START
// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""
// The name of the account under which the service should run
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"
// The password to the service account name
#define SERVICE_PASSWORD         NULL

class AppService : public System::WinServiceBase
{
private:
    std::shared_ptr<Application> app_;
    std::thread thread_;
protected:
    void OnStart(DWORD dwArgc, PWSTR* pszArgv) override;
    void OnStop() override;
    void OnShutdown() override;
public:
    AppService();
    ~AppService() override = default;
};

#endif
