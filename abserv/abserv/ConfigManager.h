#pragma once

#include <string>
#include "Variant.h"
#include <lua.hpp>

class ConfigManager
{
public:
    enum Key
    {
        Location,
        IP,
        GamePort,
        AdminPort,
        LoginPort,
        StatusPort,

        DBHost,
        DBPort,
        DBUser,
        DBPass
    };
private:
    lua_State* L;
    bool isLoaded;
    std::string GetGlobal(const std::string& ident, const std::string& default);
    int64_t GetGlobal(const std::string& ident, int64_t default);
public:
    ConfigManager();
    ~ConfigManager()
    {
        if (L)
            lua_close(L);
    }

    bool Load(const std::string& file);
    static ConfigManager Instance;

    Utils::VariantMap config_;
};

