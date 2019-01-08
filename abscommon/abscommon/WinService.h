#pragma once

#ifdef _WIN32

#include <windows.h>
#include "ServiceInstaller.h"

namespace System {

class WinServiceBase
{
private:
    // The name of the service
    PWSTR m_name;
    // The service status handle
    SERVICE_STATUS_HANDLE m_statusHandle;
    // The status of the service
    SERVICE_STATUS m_status;
    // The singleton service instance.
    static WinServiceBase* s_service;
    // Entry point for the service. It registers the handler function for the
    // service and starts the service.
    static void WINAPI ServiceMain(DWORD dwArgc, LPWSTR* lpszArgv);
    // The function is called by the SCM whenever a control code is sent to
    // the service.
    static void WINAPI ServiceCtrlHandler(DWORD dwCtrl);
    // Start the service.
    void Start(DWORD dwArgc, PWSTR* pszArgv);
    // Pause the service.
    void Pause();
    // Resume the service after being paused.
    void Continue();
    // Execute when the system is shutting down.
    void Shutdown();
protected:
    // When implemented in a derived class, executes when a Start command is
    // sent to the service by the SCM or when the operating system starts
    // (for a service that starts automatically). Specifies actions to take
    // when the service starts.
    virtual void OnStart(DWORD /* dwArgc */, PWSTR* /* pszArgv */) { }
    // When implemented in a derived class, executes when a Stop command is
    // sent to the service by the SCM. Specifies actions to take when a
    // service stops running.
    virtual void OnStop() { }
    // When implemented in a derived class, executes when a Pause command is
    // sent to the service by the SCM. Specifies actions to take when a
    // service pauses.
    virtual void OnPause() { }
    // When implemented in a derived class, OnContinue runs when a Continue
    // command is sent to the service by the SCM. Specifies actions to take
    // when a service resumes normal functioning after being paused.
    virtual void OnContinue() { }
    // When implemented in a derived class, executes when the system is
    // shutting down. Specifies what should occur immediately prior to the
    // system shutting down.
    virtual void OnShutdown() { }

    // Set the service status and report the status to the SCM.
    void SetServiceStatus(DWORD dwCurrentState,
        DWORD dwWin32ExitCode = NO_ERROR,
        DWORD dwWaitHint = 0);

    // Log a message to the Application event log.
    void WriteEventLogEntry(PWSTR pszMessage, WORD wType);

    // Log an error message to the Application event log.
    void WriteErrorLogEntry(PWSTR pszFunction,
        DWORD dwError = GetLastError());
public:
    // Register the executable for a service with the Service Control Manager
    // (SCM). After you call Run(ServiceBase), the SCM issues a Start command,
    // which results in a call to the OnStart method in the service. This
    // method blocks until the service has stopped.
    static BOOL Run(WinServiceBase& service);

    WinServiceBase(PWSTR pszServiceName,
        BOOL fCanStop = TRUE,
        BOOL fCanShutdown = TRUE,
        BOOL fCanPauseContinue = FALSE);
    virtual ~WinServiceBase();
    // Stop the service.
    void Stop();
};

template <typename T>
class WinService : public WinServiceBase
{
private:
    std::shared_ptr<T> app_;
    std::thread thread_;
protected:
    void OnStart(DWORD dwArgc, PWSTR* pszArgv) override
    {
        app_ = std::make_shared<T>();
        if (!app_->InitializeW(dwArgc, pszArgv))
        {
            WriteEventLogEntry(L"Application initialization failed", EVENTLOG_ERROR_TYPE);
            app_.reset();
            return;
        }
        thread_ = std::thread(&T::Run, app_);
    }
    void OnStop() override
    {
        if (app_)
        {
            app_->Stop();
            app_.reset();
            thread_.join();
        }
    }
    void OnShutdown() override
    {
        if (app_)
        {
            app_->Stop();
            app_.reset();
            thread_.join();
        }
    }
public:
    WinService() :
        WinServiceBase(SERVICE_NAME)
    { }
    ~WinService() override = default;
};

}

// Creates the entry point
#define AB_SERVICE_MAIN(serviceName)                                           \
int wmain(int argc, wchar_t* argv[])                                           \
{                                                                              \
    if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))              \
    {                                                                          \
        if (_wcsicmp(L"install", argv[1] + 1) == 0)                            \
        {                                                                      \
            System::InstallService(                                            \
                SERVICE_NAME,                                                  \
                SERVICE_DISPLAY_NAME,                                          \
                SERVICE_DESCRIPTION,                                           \
                SERVICE_START_TYPE,                                            \
                SERVICE_DEPENDENCIES,                                          \
                SERVICE_ACCOUNT,                                               \
                SERVICE_PASSWORD                                               \
            );                                                                 \
        }                                                                      \
        else if (_wcsicmp(L"remove", argv[1] + 1) == 0)                        \
            System::UninstallService(SERVICE_NAME);                            \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        wprintf(L"Parameters:\n");                                             \
        wprintf(L" -install  to install the service.\n");                      \
        wprintf(L" -remove   to remove the service.\n");                       \
        serviceName service;                                                   \
        if (!System::WinServiceBase::Run(service))                             \
            wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError()); \
        return EXIT_FAILURE;                                                   \
    }                                                                          \
    return EXIT_SUCCESS;                                                       \
}                                                                              \

#endif
