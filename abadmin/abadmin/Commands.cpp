#include "stdafx.h"
#include "Commands.h"
#include "Client.h"
#include <stdint.h>
#include <iostream>

Commands Commands::Instance;
extern Client* gClient;
extern bool gRunning;

std::string Trim(const std::string& str,
    const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

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
    if (gClient->IsConnected())
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
    gClient->Connect(pass);

    int loops = 0;
    while (!gClient->IsLoggedIn())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ++loops;
        // Wait max 5sec
        if (loops > 100)
            break;
    }

    if (gClient->IsLoggedIn())
        std::cout << "Logged in" << std::endl;
    else
        std::cout << "Not logged in" << std::endl;
    return 1;
}

int CommandDisconnect(const std::vector<std::string>& params)
{
    if (!gClient->IsConnected())
    {
        std::cout << "Not connected" << std::endl;
        return -1;
    }
    gClient->Disconnect();
    return 1;
}

int CommandBroadcast(const std::vector<std::string>& params)
{
    if (!gClient->IsConnected())
    {
        std::cout << "Not connected" << std::endl;
        return -1;
    }
    if (params.size() < 1)
    {
        std::cout << "Missing parameters" << std::endl;
        return -1;
    }

    std::string msg;
    for (const auto& m : params)
        msg += " " + m;

    msg = Trim(msg);
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
        return -1;
    }
    return 1;
}

int CommandCloseServer(const std::vector<std::string>& params)
{
    if (!gClient->IsConnected())
    {
        std::cout << "Not connected" << std::endl;
        return -1;
    }
    bool ret = gClient->SendCommand(CMD_CLOSE_SERVER, nullptr);
    if (!ret)
    {
        return -1;
    }
    return 1;
}

int CommandShutdownServer(const std::vector<std::string>& params)
{
    if (!gClient->IsConnected())
    {
        std::cout << "Not connected" << std::endl;
        return -1;
    }
    bool ret = gClient->SendCommand(CMD_SHUTDOWN_SERVER, nullptr);
    if (!ret)
    {
        return -1;
    }
//    gClient->Disconnect();
    return 1;
}

int CommandKickPlayer(const std::vector<std::string>& params)
{
    if (!gClient->IsConnected())
    {
        std::cout << "Not connected" << std::endl;
        return -1;
    }
    if (params.size() < 1)
    {
        std::cout << "Missing parameters" << std::endl;
        return -1;
    }
    std::string name = params[0];
    bool ret = gClient->SendCommand(CMD_KICK, (char*)name.c_str());

    return ret ? 1 : -1;
}

void Commands::Initialize()
{
    commands_["q"] = Command(&QuitCommand, "\n  Quit");
    commands_["h"] = Command(&HelpCommand, "\n  Show help");
    commands_["server"] = Command(&CommandServer, "<host> <port>\n  Set server");
    commands_["connect"] = Command(&CommandConnect, "[<pass>]\n  Connect to server");
    commands_["disconnect"] = Command(&CommandDisconnect, "\n  Disconnect from server");
    commands_["broadcast"] = Command(&CommandBroadcast, "<message>\n  Broadcast a message");
    commands_["close"] = Command(&CommandCloseServer, "\n  Close server");
    commands_["shutdown"] = Command(&CommandShutdownServer, "\n  Shutdown server");
    commands_["kick"] = Command(&CommandKickPlayer, "<name>\n  Kick player");
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
