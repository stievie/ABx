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
#include "ConfigManager.h"

ConfigManager::ConfigManager() :
    IO::SimpleConfigManager()
{
    config_[Key::ServerName] = "abserv";
    config_[Key::GamePort] = 0;
    config_[Key::GameIP] = 0; // INADDR_ANY

    config_[Key::DataServerHost] = "localhost";
    config_[Key::DataServerPort] = 2770;
}

bool ConfigManager::Load(const std::string& file)
{
    if (!IO::SimpleConfigManager::Load(file))
        return false;

    config_[Key::Machine] = GetGlobalString("machine", "");
    config_[Key::ServerName] = GetGlobalString("server_name", "abserv");
    config_[Key::ServerID] = GetGlobalString("server_id", Utils::Uuid::EMPTY_UUID);
    config_[Key::Location] = GetGlobalString("location", "--");
    config_[Key::GameIP] = Utils::ConvertStringToIP(GetGlobalString("game_ip", "0.0.0.0"));
    config_[Key::LogDir] = GetGlobalString("log_dir", "");
    config_[Key::DataDir] = GetGlobalString("data_dir", "");
    config_[Key::RecordingsDir] = GetGlobalString("recordings_dir", "");
    config_[Key::RecordGames] = GetGlobalBool("record_games", false);
    config_[Key::GamePort] = static_cast<int>(GetGlobalInt("game_port", 0ll));
    config_[Key::GameHost] = GetGlobalString("game_host", "");
    config_[Key::ServerKeys] = GetGlobalString("server_keys", "");

    config_[Key::DataServerHost] = GetGlobalString("data_host", "localhost");
    config_[Key::DataServerPort] = static_cast<int>(GetGlobalInt("data_port", 2770ll));
    config_[Key::MessageServerHost] = GetGlobalString("message_host", "localhost");
    config_[Key::MessageServerPort] = static_cast<int>(GetGlobalInt("message_port", 2771ll));

    config_[Key::MaxPacketsPerSecond] = static_cast<int>(GetGlobalInt("max_packets_per_second", 25ll));

    config_[Key::Behaviours] = GetGlobalString("behaviours", "/scripts/behaviors/behaviors.lua");
    config_[Key::AiServer] = GetGlobalBool("ai_server", false);
    config_[Key::AiServerIp] = GetGlobalString("ai_server_ip", "127.0.0.1");
    config_[Key::AiServerPort] = static_cast<int>(GetGlobalInt("ai_server_port", 12345ll));
    config_[Key::AiUpdateInterval] = static_cast<int>(GetGlobalInt("ai_server_interval", 1000ll));

    Close();
    return true;
}
