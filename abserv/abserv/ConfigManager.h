#pragma once

#include "Variant.h"
#include <lua.hpp>
#include "SimpleConfigManager.h"

class ConfigManager : public IO::SimpleConfigManager
{
public:
    enum Key
    {
        ServerName,
        ServerID,
        Location,
        IP,
        GamePort,
        GameHost,
        FilePort,
        FileHost,
        AdminPort,
        LoginPort,
        StatusPort,
        LoginIP,
        GameIP,
        StatusIP,
        AdminIP,
        LogDir,
        DataDir,

        DataServerHost,
        DataServerPort,

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

        PlayerLevelCap,
    };
public:
    ConfigManager();
    ~ConfigManager() = default;

    Utils::Variant& operator[](Key key)
    {
        return config_[key];
    }

    bool Load(const std::string& file);
    static ConfigManager Instance;

    Utils::VariantMap config_;
};

