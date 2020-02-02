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
#include "Application.h"
#include "ServiceConfig.h"
#if !defined(WIN_SERVICE)
#include <csignal>
#include "MiniDump.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#   define CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#endif

namespace {
std::function<void(int)> shutdown_handler;
void signal_handler(int signal)
{
    shutdown_handler(signal);
}
} // namespace

#ifdef AB_WINDOWS
static std::mutex gTermLock;
static std::condition_variable termSignal;
#endif

int main(int argc, char** argv)
{
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#if defined(AB_WINDOWS) && defined(WRITE_MINIBUMP)
    SetUnhandledExceptionFilter(System::UnhandledHandler);
#endif

    std::signal(SIGINT, signal_handler);              // Ctrl+C
    std::signal(SIGTERM, signal_handler);
#ifdef AB_WINDOWS
    std::signal(SIGBREAK, signal_handler);            // X clicked
#endif

    {
        Application app;
        if (!app.InitializeA(argc, argv))
            return EXIT_FAILURE;

        shutdown_handler = [&app](int /*signal*/)
        {
#ifdef AB_WINDOWS
            std::unique_lock<std::mutex> lockUnique(gTermLock);
#endif
            app.Stop();
#ifdef AB_WINDOWS
            termSignal.wait(lockUnique);
#endif
        };

        app.Run();
    }

#ifdef AB_WINDOWS
    termSignal.notify_all();
#endif

    return EXIT_SUCCESS;
}

#else   // !defined(WIN_SERVICE)
// Internal name of the service
#define SERVICE_NAME             L"ABMatchServer"
// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"AB Match Server"
#define SERVICE_DESCRIPTION      L"Forgotten Wars Match Server"
// Service start options.
#define SERVICE_START_TYPE       SERVICE_DEMAND_START
#define SERVICE_DEPENDENCIES     L"ABDataServer\0ABMessageServer\0\0"
// The name of the account under which the service should run
// LocalService may not start because it does not have access to the directory
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"
// The password to the service account name
#define SERVICE_PASSWORD         NULL

#include "WinService.h"
AB_SERVICE_MAIN(System::WinService<Application>)
#endif // !defined(WIN_SERVICE)
