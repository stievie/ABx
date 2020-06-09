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

#include <iostream>
#include <sa/ArgParser.h>
#include <sa/Compiler.h>
#include <AB/CommonConfig.h>
PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4592)
    PRAGMA_WARNING_DISABLE_CLANG("-Wpadded")
    PRAGMA_WARNING_DISABLE_GCC("-Wpadded")
#   include <asio.hpp>
PRAGMA_WARNING_POP
#include <abscommon/Subsystems.h>
#include <abscommon/Scheduler.h>
#include <abscommon/Dispatcher.h>
#include <thread>
#include <atomic>
#include "DebugClient.h"
#include "Window.h"
#include <thread>
#include <csignal>

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << sa::arg_parser::get_help("dbgclient", _cli, "Debug Client");
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

    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();

    asio::io_service io;

    Window wnd;
    DebugClient client(io, wnd);

    std::string host = sa::arg_parser::get_value<std::string>(parsedArgs, "host", "");
    uint16_t port = sa::arg_parser::get_value<uint16_t>(parsedArgs, "port", 0);
    if (!client.Connect(host, port))
    {
        std::cerr << "Unable to connect to " << host << ":" << port << std::endl;
        return EXIT_FAILURE;
    }

    std::thread thread([&io]() { io.run(); });
    wnd.Loop();
    io.stop();
    thread.join();

    GetSubsystem<Asynch::Dispatcher>()->Stop();
    GetSubsystem<Asynch::Scheduler>()->Stop();

    return EXIT_SUCCESS;
}
