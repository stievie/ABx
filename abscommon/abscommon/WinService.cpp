/**
 * Copyright 2017-2020 Stefan Ascher
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
#include "WinService.h"

#ifdef AB_WINDOWS

#include <strsafe.h>

namespace System {

// Initialize the singleton service instance.
WinServiceBase* WinServiceBase::s_service = NULL;

void WinServiceBase::ServiceMain(DWORD dwArgc, LPWSTR* lpszArgv)
{
    assert(s_service != NULL);

    // Register the handler function for the service
    s_service->m_statusHandle = RegisterServiceCtrlHandler(
        s_service->m_name, ServiceCtrlHandler);
    if (s_service->m_statusHandle == NULL)
    {
        throw GetLastError();
    }

    // Start the service.
    s_service->Start(dwArgc, lpszArgv);
}

void WinServiceBase::ServiceCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        s_service->Stop();
        break;
    case SERVICE_CONTROL_PAUSE:
        s_service->Pause();
        break;
    case SERVICE_CONTROL_CONTINUE:
        s_service->Continue();
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        s_service->Shutdown();
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    default:
        break;
    }
}

void WinServiceBase::Start(DWORD dwArgc, PWSTR* pszArgv)
{
    try
    {
        // Tell SCM that the service is starting.
        SetServiceStatus(SERVICE_START_PENDING);

        // Perform service-specific initialization.
        if (OnStart(dwArgc, pszArgv))
            // Tell SCM that the service is started.
            SetServiceStatus(SERVICE_RUNNING);
        else
            SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD dwError)
    {
        // Log the error.
        WriteErrorLogEntry(L"Service Start", dwError);

        // Set the service status to be stopped.
        SetServiceStatus(SERVICE_STOPPED, dwError);
    }
    catch (...)
    {
        // Log the error.
        WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);

        // Set the service status to be stopped.
        SetServiceStatus(SERVICE_STOPPED);
    }
}

void WinServiceBase::Pause()
{
    try
    {
        // Tell SCM that the service is pausing.
        SetServiceStatus(SERVICE_PAUSE_PENDING);

        // Perform service-specific pause operations.
        OnPause();

        // Tell SCM that the service is paused.
        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (DWORD dwError)
    {
        // Log the error.
        WriteErrorLogEntry(L"Service Pause", dwError);

        // Tell SCM that the service is still running.
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (...)
    {
        // Log the error.
        WriteEventLogEntry(L"Service failed to pause.", EVENTLOG_ERROR_TYPE);

        // Tell SCM that the service is still running.
        SetServiceStatus(SERVICE_RUNNING);
    }
}

void WinServiceBase::Continue()
{
    try
    {
        // Tell SCM that the service is resuming.
        SetServiceStatus(SERVICE_CONTINUE_PENDING);

        // Perform service-specific continue operations.
        OnContinue();

        // Tell SCM that the service is running.
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD dwError)
    {
        // Log the error.
        WriteErrorLogEntry(L"Service Continue", dwError);

        // Tell SCM that the service is still paused.
        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (...)
    {
        // Log the error.
        WriteEventLogEntry(L"Service failed to resume.", EVENTLOG_ERROR_TYPE);

        // Tell SCM that the service is still paused.
        SetServiceStatus(SERVICE_PAUSED);
    }
}

void WinServiceBase::Shutdown()
{
    try
    {
        // Perform service-specific shutdown operations.
        OnShutdown();

        // Tell SCM that the service is stopped.
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD dwError)
    {
        // Log the error.
        WriteErrorLogEntry(L"Service Shutdown", dwError);
    }
    catch (...)
    {
        // Log the error.
        WriteEventLogEntry(L"Service failed to shut down.", EVENTLOG_ERROR_TYPE);
    }
}

void WinServiceBase::SetServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure of the service.

    m_status.dwCurrentState = dwCurrentState;
    m_status.dwWin32ExitCode = dwWin32ExitCode;
    m_status.dwWaitHint = dwWaitHint;

    m_status.dwCheckPoint =
        ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED)) ?
        0 : dwCheckPoint++;

    // Report the status of the service to the SCM.
    ::SetServiceStatus(m_statusHandle, &m_status);
}

void WinServiceBase::WriteEventLogEntry(PWSTR pszMessage, WORD wType)
{
    HANDLE hEventSource = NULL;
    LPCWSTR lpszStrings[2] = { NULL, NULL };

    hEventSource = RegisterEventSource(NULL, m_name);
    if (hEventSource)
    {
        lpszStrings[0] = m_name;
        lpszStrings[1] = pszMessage;

        ReportEvent(hEventSource,  // Event log handle
            wType,                 // Event type
            0,                     // Event category
            0,                     // Event identifier
            NULL,                  // No security identifier
            2,                     // Size of lpszStrings array
            0,                     // No binary data
            lpszStrings,           // Array of strings
            NULL                   // No binary data
        );

        DeregisterEventSource(hEventSource);
    }
}

void WinServiceBase::WriteErrorLogEntry(PWSTR pszFunction, DWORD dwError)
{
    wchar_t szMessage[260];
    StringCchPrintf(szMessage, ARRAYSIZE(szMessage),
        L"%s failed w/err 0x%08lx", pszFunction, dwError);
    WriteEventLogEntry(szMessage, EVENTLOG_ERROR_TYPE);
}

BOOL WinServiceBase::Run(WinServiceBase& service)
{
    s_service = &service;

    SERVICE_TABLE_ENTRY serviceTable[] =
    {
        { service.m_name, ServiceMain },
        { NULL, NULL }
    };

    // Connects the main thread of a service process to the service control
    // manager, which causes the thread to be the service control dispatcher
    // thread for the calling process. This call returns when the service has
    // stopped. The process should simply terminate when the call returns.
    return StartServiceCtrlDispatcher(serviceTable);
}

WinServiceBase::WinServiceBase(PWSTR pszServiceName,
    BOOL fCanStop /* = TRUE */,
    BOOL fCanShutdown /* = TRUE */,
    BOOL fCanPauseContinue /* = FALSE */)
{
    // Service name must be a valid string and cannot be NULL.
    m_name = (pszServiceName == NULL) ? L"" : pszServiceName;

    m_statusHandle = NULL;

    // The service runs in its own process.
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

    // The service is starting.
    m_status.dwCurrentState = SERVICE_START_PENDING;

    // The accepted commands of the service.
    DWORD dwControlsAccepted = 0;
    if (fCanStop)
        dwControlsAccepted |= SERVICE_ACCEPT_STOP;
    if (fCanShutdown)
        dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
    if (fCanPauseContinue)
        dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
    m_status.dwControlsAccepted = dwControlsAccepted;

    m_status.dwWin32ExitCode = NO_ERROR;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}


WinServiceBase::~WinServiceBase()
{
}

void WinServiceBase::Stop()
{
    DWORD dwOriginalState = m_status.dwCurrentState;
    try
    {
        // Tell SCM that the service is stopping.
        SetServiceStatus(SERVICE_STOP_PENDING);

        // Perform service-specific stop operations.
        OnStop();

        // Tell SCM that the service is stopped.
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD dwError)
    {
        // Log the error.
        WriteErrorLogEntry(L"Service Stop", dwError);

        // Set the original service status.
        SetServiceStatus(dwOriginalState);
    }
    catch (...)
    {
        // Log the error.
        WriteEventLogEntry(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);

        // Set the original service status.
        SetServiceStatus(dwOriginalState);
    }
}

}

#endif
