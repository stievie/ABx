#include <iostream>
#include <sa/ArgParser.h>
#include <sa/PragmaWarning.h>
#include <AB/CommonConfig.h>
PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4592)
    PRAGMA_WARNING_DISABLE_CLANG("-Wpadded")
    PRAGMA_WARNING_DISABLE_GCC("-Wpadded")
#   include <asio.hpp>
PRAGMA_WARNING_POP
#include "Subsystems.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include <thread>
#include <atomic>
#include "DebugClient.h"
#include "Window.h"

static asio::io_service io;
static std::atomic<bool> running = false;

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << std::endl;
    std::cout << sa::arg_parser::get_help("dbgclient", _cli);
}

static void RunIo()
{
    io.run();
    if (running)
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(500, std::bind(&RunIo)));
}

int main(int argc, char** argv)
{
    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none },
        { "host", { "-host" }, "Server host", true, true, sa::arg_parser::option_type::string },
        { "port", { "-p", "-port" }, "Server port", true, true, sa::arg_parser::option_type::integer }
    } };

    sa::arg_parser::values parsedArgs;
    sa::arg_parser::result cmdres = sa::arg_parser::parse(argc, argv, _cli, parsedArgs);
    auto val = sa::arg_parser::get_value<bool>(parsedArgs, "help");
    if (val.has_value() && val.value())
    {
        ShowHelp(_cli);
        return EXIT_SUCCESS;
    }
    if (!cmdres)
    {
        std::cerr << cmdres << std::endl;
        std::cerr << "Type `dbgclient -h` for help." << std::endl;
        return EXIT_FAILURE;
    }

    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();

    DebugClient client(io);

    std::string host = sa::arg_parser::get_value<std::string>(parsedArgs, "host", "");
    uint16_t port = sa::arg_parser::get_value<uint16_t>(parsedArgs, "port", 0);
    if (!client.Connect(host, port))
    {
        std::cerr << "Unable to connect to " << host << ":" << port << std::endl;
        return EXIT_FAILURE;
    }

    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();
    running = true;
    GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(100, std::bind(&RunIo)));

    Window wnd;
    wnd.Loop();
    running = false;

    GetSubsystem<Asynch::Dispatcher>()->Stop();
    GetSubsystem<Asynch::Scheduler>()->Stop();

    return EXIT_SUCCESS;
}
