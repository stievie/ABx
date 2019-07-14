#include "stdafx.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "StringUtils.h"
#include "UuidUtils.h"

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

    config_[Key::LoginTries] = static_cast<int>(GetGlobalInt("login_tries", 5ll));
    config_[Key::LoginTimeout] = static_cast<int>(GetGlobalInt("login_timeout", 60ll * 1000ll));
    config_[Key::LoginRetryTimeout] = static_cast<int>(GetGlobalInt("login_retrytimeout", 5000ll));

    config_[Key::Behaviours] = GetGlobalString("behaviours", "/scripts/ai/behaviours.lua");
    config_[Key::AIServer] = GetGlobalBool("ai_server", false);
    config_[Key::AIServerIp] = GetGlobalString("ai_server_ip", "0.0.0.0");
    config_[Key::AIServerPort] = static_cast<int>(GetGlobalInt("ai_server_port", 10001));

    Close();
    return true;
}
