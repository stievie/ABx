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


#include "ServerApp.h"
#if defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <linux/limits.h>
#endif
#include "DataClient.h"
#include "MessageClient.h"
#include "Process.hpp"
#include "StringUtils.h"
#include "UuidUtils.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include <algorithm>
#include <codecvt>
#include <sa/StringTempl.h>
#include "FileUtils.h"
#include <sa/PropStream.h>

ServerApp::ServerApp() :
    running_(false),
    serverType_(AB::Entities::ServiceTypeUnknown),
    serverId_(Utils::Uuid::EMPTY_UUID),
    machine_(""),
    serverName_(""),
    serverLocation_(""),
    serverPort_(std::numeric_limits<uint16_t>::max())
{
    cli_.push_back({ "help", { "-h", "-help", "-?" }, "Show this help", false, false, sa::arg_parser::option_type::none });
    cli_.push_back({ "version", { "-v", "--version" }, "Show program version", false, false, sa::arg_parser::option_type::none });
    cli_.push_back({ "nologo", { "--no-logo" }, "Do not show logo at program start", false, false, sa::arg_parser::option_type::none });
    cli_.push_back({ "config", { "-conf", "--config-file" }, "Config file", false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "logdir", { "-log", "--log-dir" }, "Directory to store log files", false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "id", { "-id", "--server-id" }, "Server UUID", false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "machine", { "-machine", "--machine-name" }, "Machine name", false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "ip", { "-ip", "--ip-address" }, "IP Address to listen on. 0.0.0.0 to listen on all.", false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "port", { "-port", "--port" }, "Port it listens on. If 0 it uses a random free port.", false, true, sa::arg_parser::option_type::integer });
    cli_.push_back({ "host", { "-host", "--host-name" }, "Host name", false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "name", { "-name", "--server-name" }, "Name of the Server. If `generic` it generates a name.", false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "loc", { "-loc", "--server-location" }, "Location of the Server.", false, true, sa::arg_parser::option_type::string });
}

std::string ServerApp::GetFreeName(IO::DataClient* client)
{
    AB::Entities::ServiceList sl;
    if (!client->Read(sl))
        return std::string();

    std::vector<std::string> names;
    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!client->Read(s))
            continue;
        // Exact match so return this name
        if (Utils::Uuid::IsEqual(s.uuid, serverId_))
            return s.name;
        names.push_back(s.name);
    }

    std::string prefix = serverLocation_;
    switch (serverType_)
    {
    case AB::Entities::ServiceTypeDataServer:
        prefix += "D";
        break;
    case AB::Entities::ServiceTypeMessageServer:
        prefix += "M";
        break;
    case AB::Entities::ServiceTypeFileServer:
        prefix += "F";
        break;
    case AB::Entities::ServiceTypeLoginServer:
        prefix += "L";
        break;
    case AB::Entities::ServiceTypeGameServer:
        // No type prefix for game server: AT1, AT2
        break;
    case AB::Entities::ServiceTypeAdminServer:
        // Different!
        prefix = "AB Admin " + serverLocation_ + " ";
        break;
    case AB::Entities::ServiceTypeMatchServer:
        prefix += "MM";
        break;
    case AB::Entities::ServiceTypeLoadBalancer:
        prefix += "LB";
        break;
    default:
        prefix += "?";
        break;
    }

    unsigned i = 1;
    std::string result = prefix + std::to_string(i);
    while (std::find(names.begin(), names.end(), result) != names.end())
    {
        ++i;
        result = prefix + std::to_string(i);
    }
    return result;
}

bool ServerApp::SendServerJoined(Net::MessageClient* client, const AB::Entities::Service& service)
{
    if (!client)
        return false;

    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::ServerJoined;

    sa::PropWriteStream stream;
    stream.Write<AB::Entities::ServiceType>(service.type);
    stream.WriteString(service.uuid);
    stream.WriteString(service.host);
    stream.Write<uint16_t>(service.port);
    stream.WriteString(service.location);
    stream.WriteString(service.name);
    stream.WriteString(service.machine);
    msg.SetPropStream(stream);
    return client->Write(msg);
}

bool ServerApp::SendServerLeft(Net::MessageClient* client, const AB::Entities::Service& service)
{
    if (!client)
        return false;

    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::ServerLeft;
    sa::PropWriteStream stream;
    stream.Write<AB::Entities::ServiceType>(service.type);
    stream.WriteString(service.uuid);
    stream.WriteString(service.host);
    stream.Write<uint16_t>(service.port);
    stream.WriteString(service.location);
    stream.WriteString(service.name);
    stream.WriteString(service.machine);
    msg.SetPropStream(stream);
    return client->Write(msg);
}

void ServerApp::UpdateService(AB::Entities::Service& service)
{
    if (service.machine.empty())
        service.machine = machine_;
    service.location = serverLocation_;
    service.host = serverHost_;
    service.port = serverPort_;
    service.ip = serverIp_;
    service.name = serverName_;
    service.file = exeFile_;
    service.path = path_;
    service.arguments = sa::CombineString(arguments_, std::string(" "));
    service.type = serverType_;
}

void ServerApp::ShowHelp()
{
    std::cout << sa::arg_parser::get_help(Utils::ExtractFileName(exeFile_), cli_);
}

void ServerApp::ShowCommandlineError(const sa::arg_parser::result& err)
{
    std::cout << err << std::endl;
    std::cout << "Type `" << Utils::ExtractFileName(exeFile_) << " -h` for help." << std::endl;
}

bool ServerApp::ParseCommandLine()
{
    auto cmdres = sa::arg_parser::parse(arguments_, cli_, parsedArgs_);
    if (!cmdres)
    {
        ShowCommandlineError(cmdres);
        return false;
    }
    if (sa::arg_parser::get_value<bool>(parsedArgs_, "help", false))
    {
        // -help was there
        ShowHelp();
        return false;
    }
    if (sa::arg_parser::get_value<bool>(parsedArgs_, "version", false))
    {
        // -help was there
        ShowVersion();
        return false;
    }

    configFile_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "config", configFile_);
    logDir_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "logdir", logDir_);
    auto idval = sa::arg_parser::get_value<std::string>(parsedArgs_, "id");
    if (idval.has_value())
    {
        // There was an argument
        serverId_ = idval.value();
        if (uuids::uuid(serverId_).nil())
        {
            serverId_ = Utils::Uuid::New();
            LOG_INFO << "Generating new Server ID " << serverId_ << std::endl;
        }
    }
    machine_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "machine", machine_);
    serverIp_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "ip", serverIp_);
    serverHost_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "host", serverHost_);
    serverPort_ = sa::arg_parser::get_value<uint16_t>(parsedArgs_, "port", serverPort_);
    serverName_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "name", serverName_);
    serverLocation_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "log", serverLocation_);

    return true;
}

void ServerApp::Init()
{
    exeFile_ = Utils::GetExeName();
    path_ = Utils::ExtractFileDir(exeFile_);
    machine_ = GetMachineName();
}

std::string ServerApp::GetMachineName()
{
#ifdef AB_WINDOWS
    char buff[256];
    DWORD size = sizeof(buff);
    BOOL ret = GetComputerNameExA(ComputerNameDnsHostname, buff, &size);
    if (ret == 0)
    {
        size = sizeof(buff);
        ZeroMemory(buff, size);
        ret = GetComputerNameA(buff, &size);
    }
    if (ret != 0)
        return std::string(buff, size);
    return "";
#else
    char buff[64];
    if (gethostname(buff, 64) == 0)
        return std::string(buff);
    return "";
#endif
}

bool ServerApp::InitializeW(int argc, wchar_t** argv)
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
    {
        char buffer[500];
        // First arg is the pointer to destination char, second arg is
        // the pointer to source wchar_t, last arg is the size of char buffer
        wcstombs(buffer, argv[i], 500);
        args.push_back(buffer);
    }

    return Initialize(args);
}

bool ServerApp::InitializeA(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }

    return Initialize(args);
}

bool ServerApp::Initialize(const std::vector<std::string>& args)
{
    Init();
    arguments_ = args;

    return true;
}

void ServerApp::Spawn(const std::string& additionalArguments)
{
    std::stringstream ss;
    ss << Utils::EscapeArguments(exeFile_);
    // 1. Use same config file
    // 2. Use dynamic server ID
    // 3. Use generic server name
    // 4. Use random free port
    ss << " -conf " << Utils::EscapeArguments(configFile_) << " -id 00000000-0000-0000-0000-000000000000 -name generic -port 0";
    if (!logDir_.empty())
        ss << " -log " << Utils::EscapeArguments(logDir_);
    if (!serverIp_.empty())
        ss << " -ip " << serverIp_;
    if (!serverHost_.empty())
        ss << " -host " << Utils::EscapeArguments(serverHost_);
    if (!machine_.empty())
        ss << " -machine " << Utils::EscapeArguments(machine_);
    if (!additionalArguments.empty())
        ss << " " << additionalArguments;

    const std::string cmdLine = ss.str();
#ifdef AB_WINDOWS
#if defined(UNICODE)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wcmdLine = converter.from_bytes(cmdLine);
    System::Process process(wcmdLine);
#else
    System::Process process(cmdLine);
#endif
#else
    System::Process process(cmdLine);
#endif
}
