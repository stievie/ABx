#include "stdafx.h"
#include "ConfigManager.h"
#include "Logger.h"

#include "DebugNew.h"

ConfigManager ConfigManager::Instance;

ConfigManager::ConfigManager() :
    L(nullptr),
    isLoaded(false)
{
    config_[Key::ServerName] = "abserv";
    config_[Key::LoginPort] = 1336;
    config_[Key::AdminPort] = 1336;
    config_[Key::StatusPort] = 1336;
    config_[Key::GamePort] = 1337;
    config_[Key::AdminEnabled] = false;
    config_[Key::AdminLocalhostOnly] = true;
}

std::string ConfigManager::GetGlobal(const std::string& ident, const std::string& default)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return default;
    }

    int len = (int)lua_strlen(L, -1);
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
    L = lua_open();
    if (!L)
        return false;
    luaL_openlibs(L);

    if (luaL_dofile(L, file.c_str()) != 0)
    {
        int len = (int)lua_strlen(L, -1);
        std::string err(lua_tostring(L, -1), len);
        LOG_ERROR << err << std::endl;
        lua_close(L);
        L = nullptr;
        return false;
    }

    config_[Key::ServerName] = GetGlobal("server_name", "abserv");
    config_[Key::Location] = GetGlobal("location", "Unknown");
    config_[Key::IP] = GetGlobal("ip", "127.0.01");
    config_[Key::LoginPort] = (int)GetGlobal("login_port", 1336);
    config_[Key::AdminPort] = (int)GetGlobal("admin_port", 1336);
    config_[Key::StatusPort] = (int)GetGlobal("status_port", 1336);
    config_[Key::GamePort] = (int)GetGlobal("game_port", 1337);

    config_[Key::DBHost] = GetGlobal("db_host", "localhost");
    config_[Key::DBPort] = (int)GetGlobal("db_port", 3306);
    config_[Key::DBUser] = GetGlobal("db_user", "root");
    config_[Key::DBPass] = GetGlobal("db_pass", "");

    config_[Key::StatusQueryTimeout] = GetGlobal("status_timeout", 30 * 1000);

    config_[Key::AdminEnabled] = GetGlobalBool("admin_enabled", false);
    config_[Key::AdminLocalhostOnly] = GetGlobalBool("admin_localhost_only", true);

    isLoaded = true;
    return true;
}
