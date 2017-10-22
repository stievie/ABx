#include "stdafx.h"
#include "Commands.h"
#include "Client.h"
#include <stdint.h>
#include <iostream>

Commands Commands::Instance;
extern Client* gClient;
extern bool gRunning;

int QuitCommand(const std::vector<std::string>& params)
{
    gRunning = false;
    return 1;
}

int HelpCommand(const std::vector<std::string>& params)
{
    for (const auto& cmd : Commands::Instance.commands_)
    {
        std::cout << cmd.first << " " << cmd.second.help_ << std::endl;
    }
    return 1;
}

int CommandServer(const std::vector<std::string>& params)
{
    if (params.size() < 2)
        return -1;
    std::string host = params[0];
    uint16_t port = std::atoi(params[1].c_str());
    gClient->SetServer(host, port);
    return 1;
}

int CommandConnect(const std::vector<std::string>& params)
{
    if (gClient->GetConnected())
    {
        std::cout << "Already connected" << std::endl;
        return -1;
    }
    if (params.size() < 1)
    {
        std::cout << "Missing parameters" << std::endl;
        return -1;
    }
    std::string pass = params[0];
    if (pass.length() > 127)
    {
        std::cout << "Password too long" << std::endl;
        return -1;
    }
    bool ret = gClient->Connect(pass);
    return ret ? 1 : -1;
}

int CoommandBroadcast(const std::vector<std::string>& params)
{
    if (!gClient->GetConnected())
    {
        std::cout << "Not connected" << std::endl;
        return -1;
    }
    if (params.size() < 1)
    {
        std::cout << "Missing parameters" << std::endl;
        return -1;
    }
    std::string msg = params[0];
    if (msg.length() > 127 || msg.length() == 0)
    {
        std::cout << "No valid message" << std::endl;
        return -1;
    }
    char message[128];
    strcpy_s(message, msg.c_str());
    bool ret = gClient->SendCommand(CMD_BROADCAST, message);
    if (!ret)
    {
        return false;
    }
    return true;
}

void Commands::Initialize()
{
    commands_["q"] = Command(&QuitCommand, "Quit");
    commands_["h"] = Command(&HelpCommand, "Show help");
    commands_["server"] = Command(&CommandServer, "<host> <port>");
    commands_["connect"] = Command(&CommandConnect, "[<pass>]");
    commands_["broadcast"] = Command(&CoommandBroadcast, "<message>");
}

int Commands::Execute(const std::string& line)
{
    std::vector<std::string> params;
    std::string cmd;
    char* input = (char*)line.c_str();
    const char* d = " ";
    char* next;
    char* pos = strtok_s(input, d, &next);
    while (pos != NULL)
    {
        if (!cmd.empty())
            params.push_back(std::string(pos));
        else
            cmd = std::string(pos);
        pos = strtok_s(NULL, d, &next);
    }

    auto it = commands_.find(cmd);
    if (it != commands_.end())
        return (*it).second.Execute(params);

    return -1;
}
