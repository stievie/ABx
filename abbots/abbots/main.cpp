#include "Application.h"
#include <abscommon/ServiceConfig.h>
#include <csignal>
#include <abscommon/MiniDump.h>
#if defined(AB_UNIX) && defined(WRITE_MINIBUMP)
#include <death_handler.h>
#endif

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
#if defined(AB_UNIX) && defined(WRITE_MINIBUMP)
    Debug::DeathHandler dh;
    dh.set_output_callback([](const char* message, size_t size) -> ssize_t
    {
        LOG_PLAIN << std::string(message, size) << std::endl;
        return static_cast<ssize_t>(size);
    });
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
