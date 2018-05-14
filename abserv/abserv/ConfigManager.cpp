#include "stdafx.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "Utils.h"

#include "DebugNew.h"

ConfigManager ConfigManager::Instance;

ConfigManager::ConfigManager() :
    L(nullptr),
    isLoaded(false)
{
    config_[Key::ServerName] = "abserv";
    config_[Key::LoginPort] = 0;
    config_[Key::AdminPort] = 0;
    config_[Key::StatusPort] = 0;
    config_[Key::GamePort] = 0;
    config_[Key::IP] = 0; // INADDR_ANY
    config_[Key::LoginIP] = 0; // INADDR_ANY
    config_[Key::GameIP] = 0; // INADDR_ANY
    config_[Key::StatusIP] = 0; // INADDR_ANY
    config_[Key::AdminIP] = 0; // INADDR_ANY
    config_[Key::AdminEnabled] = false;
    config_[Key::AdminLocalhostOnly] = true;
    config_[Key::AdminRequireEncryption] = true;

    config_[Key::DataServerHost] = "localhost";
    config_[Key::DataServerPort] = 2770;
}

std::string ConfigManager::GetGlobal(const std::string& ident, const std::string& default)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return default;
    }

    int len = (int)luaL_len(L, -1);
    std::string ret(lua_tostring(L, -1), len);
    lua_pop(L, 1);

    return ret;
}

int64_t ConfigManager::GetGlobal(const std::string& ident, int64_t default)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        return default;
    }

    int64_t val = (int64_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    return val;
}

bool ConfigManager::GetGlobalBool(const std::string& ident, bool default)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isboolean(L, -1)) {
        lua_pop(L, 1);
        return default;
    }

    bool val = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);

    return val;
}

bool ConfigManager::Load(const std::string& file)
{
    if (L)
        lua_close(L);
    L = luaL_newstate();
    if (!L)
        return false;
    luaL_openlibs(L);

    if (luaL_dofile(L, file.c_str()) != 0)
    {
        int len = (int)luaL_len(L, -1);
        std::string err(lua_tostring(L, -1), len);
        LOG_ERROR << err << std::endl;
        lua_close(L);
        L = nullptr;
        return false;
    }

    config_[Key::ServerName] = GetGlobal("server_name", "abserv");
    config_[Key::ServerID] = GetGlobal("server_id", "00000000-0000-0000-0000-000000000000");
    config_[Key::Location] = GetGlobal("location", "Unknown");
    std::string defIp = GetGlobal("ip", "0.0.0.0");
    config_[Key::IP] = Utils::ConvertStringToIP(defIp);
    config_[Key::LoginIP] = Utils::ConvertStringToIP(GetGlobal("login_ip", defIp));
    config_[Key::GameIP] = Utils::ConvertStringToIP(GetGlobal("game_ip", defIp));
    config_[Key::StatusIP] = Utils::ConvertStringToIP(GetGlobal("status_ip", defIp));
    config_[Key::AdminIP] = Utils::ConvertStringToIP(GetGlobal("admin_ip", defIp));
    config_[Key::LogDir] = GetGlobal("log_dir", "");
    config_[Key::DataDir] = GetGlobal("data_dir", "");
    config_[Key::LoginPort] = (int)GetGlobal("login_port", 0);
    config_[Key::AdminPort] = (int)GetGlobal("admin_port", 0);
    config_[Key::StatusPort] = (int)GetGlobal("status_port", 0);
    config_[Key::GamePort] = (int)GetGlobal("game_port", 0);
    config_[Key::GameHost] = GetGlobal("game_host", "");

    config_[Key::DataServerHost] = GetGlobal("data_host", "localhost");
    config_[Key::DataServerPort] = (int)GetGlobal("data_port", 2770);

    config_[Key::StatusQueryTimeout] = GetGlobal("status_timeout", 30 * 1000);
    config_[Key::MaxPacketsPerSecond] = GetGlobal("max_packets_per_second", 25);

    config_[Key::AdminEnabled] = GetGlobalBool("admin_enabled", false);
    config_[Key::AdminRequireLogin] = GetGlobalBool("admin_requirelogin", true);
    config_[Key::AdminLocalhostOnly] = GetGlobalBool("admin_localhost_only", true);
    config_[Key::AdminRequireEncryption] = GetGlobalBool("admin_require_encryption", true);
    config_[Key::AdminPassword] = GetGlobal("admin_password", "");

    config_[Key::LoginTries] = (int)GetGlobal("login_tries", 5);
    config_[Key::LoginTimeout] = (int)GetGlobal("login_timeout", 60 * 1000);
    config_[Key::LoginRetryTimeout] = (int)GetGlobal("login_retrytimeout", 5000);

    config_[Key::PlayerLevelCap] = (int)GetGlobal("level_cap", 20);

    lua_close(L);
    L = nullptr;

    isLoaded = true;
    return true;
}
