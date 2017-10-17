#include "stdafx.h"
#include "ConfigManager.h"
#include "Logger.h"

ConfigManager ConfigManager::Instance;

ConfigManager::ConfigManager() :
    L(nullptr),
    isLoaded(false)
{
    config_[Key::LoginPort] = 1336;
    config_[Key::AdminPort] = 1336;
    config_[Key::StatusPort] = 1336;
    config_[Key::GamePort] = 1337;
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

    isLoaded = true;
    return true;
}
