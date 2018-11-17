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
    config_[Key::AdminEnabled] = false;
    config_[Key::AdminLocalhostOnly] = true;
    config_[Key::AdminRequireEncryption] = true;

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
    config_[Key::GamePort] = (int)GetGlobal("game_port", (int64_t)0);
    config_[Key::GameHost] = GetGlobal("game_host", "");
    config_[Key::ServerKeys] = GetGlobal("server_keys", "");

    config_[Key::DataServerHost] = GetGlobal("data_host", "localhost");
    config_[Key::DataServerPort] = (int)GetGlobal("data_port", (int64_t)2770);
    config_[Key::MessageServerHost] = GetGlobal("message_host", "localhost");
    config_[Key::MessageServerPort] = (int)GetGlobal("message_port", (int64_t)2771);

    config_[Key::MaxPacketsPerSecond] = GetGlobal("max_packets_per_second", (int64_t)25);

    config_[Key::AdminEnabled] = GetGlobalBool("admin_enabled", false);
    config_[Key::AdminRequireLogin] = GetGlobalBool("admin_requirelogin", true);
    config_[Key::AdminLocalhostOnly] = GetGlobalBool("admin_localhost_only", true);
    config_[Key::AdminRequireEncryption] = GetGlobalBool("admin_require_encryption", true);
    config_[Key::AdminPassword] = GetGlobal("admin_password", "");

    config_[Key::LoginTries] = (int)GetGlobal("login_tries", (int64_t)5);
    config_[Key::LoginTimeout] = (int)GetGlobal("login_timeout", (int64_t)60 * 1000);
    config_[Key::LoginRetryTimeout] = (int)GetGlobal("login_retrytimeout", (int64_t)5000);

    config_[Key::PlayerLevelCap] = (int)GetGlobal("level_cap", (int64_t)20);
    config_[Key::Behaviours] = GetGlobal("behaviours", "/scripts/ai/behaviours.lua");
    config_[Key::RangeVisible] = GetGlobal("range_visible", 20.0f);

    Close();
    return true;
}
