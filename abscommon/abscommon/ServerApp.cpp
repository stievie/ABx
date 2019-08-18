#include "stdafx.h"
#include "ServerApp.h"
#include <algorithm>
#if defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <linux/limits.h>
#endif
#include "DataClient.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "StringUtils.h"
#include "MessageClient.h"
#include "UuidUtils.h"

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

    IO::PropWriteStream stream;
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
    IO::PropWriteStream stream;
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
    service.arguments = Utils::CombineString(arguments_, std::string(" "));
    service.type = serverType_;
}

bool ServerApp::ParseCommandLine()
{
    std::string value;
    if (GetCommandLineValue("-h") || GetCommandLineValue("-help"))
        return false;

    GetCommandLineValue("-conf", configFile_);
    GetCommandLineValue("-log", logDir_);
    if (GetCommandLineValue("-id", serverId_))
    {
        // There was an argument
        if (uuids::uuid(serverId_).nil())
        {
            serverId_ = Utils::Uuid::New();
            LOG_INFO << "Generating new Server ID " << serverId_ << std::endl;
        }
    }
    GetCommandLineValue("-machine", machine_);
    GetCommandLineValue("-ip", serverIp_);
    GetCommandLineValue("-host", serverHost_);
    if (GetCommandLineValue("-port", value))
    {
        serverPort_ = static_cast<uint16_t>(atoi(value.c_str()));
    }
    GetCommandLineValue("-name", serverName_);
    GetCommandLineValue("-loc", serverLocation_);
    return true;
}

bool ServerApp::GetCommandLineValue(const std::string& name, std::string& value)
{
    return Utils::GetCommandLineValue(arguments_, name, value);
}

bool ServerApp::GetCommandLineValue(const std::string& name)
{
    return Utils::GetCommandLineValue(arguments_, name);
}

void ServerApp::Init()
{
#ifdef AB_WINDOWS
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    exeFile_ = std::string(buff);
#else
    char buff[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buff, PATH_MAX);
    exeFile_ = std::string(buff, (count > 0) ? static_cast<size_t>(count) : 0);
#endif
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
