#include "stdafx.h"
#include "ServerApp.h"
#include <algorithm>
#if defined(__linux__) || defined(__unix__)
#include <unistd.h>
#endif
#include "DataClient.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "StringUtils.h"
#include "MessageClient.h"

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
        if (s.uuid.compare(serverId_) == 0)
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

bool ServerApp::Initialize(int argc, char** argv)
{
#ifdef _WIN32
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    exeFile_ = std::string(buff);
#else
    char buff[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buff, PATH_MAX);
    exeFile_ = std::string(buff, (count > 0) ? count : 0);
#endif
    path_ = Utils::ExtractFileDir(exeFile_);
    for (int i = 1; i < argc; i++)
    {
        arguments_.push_back(std::string(argv[i]));
    }

    return true;
}
