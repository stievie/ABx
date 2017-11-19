#pragma once

#include <string>
#include "Variant.h"
#include <lua.hpp>

class ConfigManager
{
public:
    enum Key
    {
        ServerName,
        Location,
        IP,
        GamePort,
        AdminPort,
        LoginPort,
        StatusPort,
        CryptoKeys,
        LogDir,
        DataDir,

        DBDriver,
        DBHost,
        DBPort,
        DBUser,
        DBPass,
        DBName,

        StatusQueryTimeout,
        MaxPacketsPerSecond,

        AdminEnabled,
        AdminRequireLogin,
        AdminLocalhostOnly,
        AdminRequireEncryption,
        AdminPassword,

        LoginTries,
        LoginTimeout,
        LoginRetryTimeout,
    };
private:
    lua_State* L;
    bool isLoaded;
    std::string GetGlobal(const std::string& ident, const std::string& default);
    int64_t GetGlobal(const std::string& ident, int64_t default);
    bool GetGlobalBool(const std::string& ident, bool default);
public:
    ConfigManager();
    ~ConfigManager()
    {
        if (L)
            lua_close(L);
    }

    Utils::Variant& operator[](Key key)
    {
        return config_[key];
    }

    bool Load(const std::string& file);
    static ConfigManager Instance;

    Utils::VariantMap config_;
};

