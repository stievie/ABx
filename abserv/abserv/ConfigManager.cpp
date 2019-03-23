#include "stdafx.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "StringUtils.h"
#include "UuidUtils.h"

#include "DebugNew.h"

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

    config_[Key::Machine] = GetGlobal("machine", "");
    config_[Key::ServerName] = GetGlobal("server_name", "abserv");
    config_[Key::ServerID] = GetGlobal("server_id", Utils::Uuid::EMPTY_UUID);
    config_[Key::Location] = GetGlobal("location", "--");
    config_[Key::GameIP] = Utils::ConvertStringToIP(GetGlobal("game_ip", "0.0.0.0"));
    config_[Key::LogDir] = GetGlobal("log_dir", "");
    config_[Key::DataDir] = GetGlobal("data_dir", "");
    config_[Key::RecordingsDir] = GetGlobal("recordings_dir", "");
    config_[Key::RecordGames] = GetGlobalBool("record_games", false);
    config_[Key::GamePort] = static_cast<int>(GetGlobal("game_port", 0ll));
    config_[Key::GameHost] = GetGlobal("game_host", "");
    config_[Key::ServerKeys] = GetGlobal("server_keys", "");

    config_[Key::DataServerHost] = GetGlobal("data_host", "localhost");
    config_[Key::DataServerPort] = static_cast<int>(GetGlobal("data_port", 2770ll));
    config_[Key::MessageServerHost] = GetGlobal("message_host", "localhost");
    config_[Key::MessageServerPort] = static_cast<int>(GetGlobal("message_port", 2771ll));

    config_[Key::MaxPacketsPerSecond] = GetGlobal("max_packets_per_second", 25ll);

    config_[Key::LoginTries] = static_cast<int>(GetGlobal("login_tries", 5ll));
    config_[Key::LoginTimeout] = static_cast<int>(GetGlobal("login_timeout", 60ll * 1000ll));
    config_[Key::LoginRetryTimeout] = static_cast<int>(GetGlobal("login_retrytimeout", 5000ll));

    config_[Key::Behaviours] = GetGlobal("behaviours", "/scripts/ai/behaviours.lua");

    Close();
    return true;
}
