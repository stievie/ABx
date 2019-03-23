#pragma once

#include "Variant.h"
#include <lua.hpp>
#include "SimpleConfigManager.h"

class ConfigManager : public IO::SimpleConfigManager
{
public:
    enum Key
    {
        Machine,
        ServerName,
        ServerID,
        Location,
        GamePort,
        GameHost,
        GameIP,
        ServerKeys,

        LogDir,
        DataDir,
        RecordingsDir,
        RecordGames,

        DataServerHost,
        DataServerPort,
        MessageServerHost,
        MessageServerPort,

        MaxPacketsPerSecond,

        LoginTries,
        LoginTimeout,
        LoginRetryTimeout,

        Behaviours,
    };
public:
    ConfigManager();
    ~ConfigManager() = default;

    Utils::Variant& operator[](Key key)
    {
        return config_[key];
    }

    bool Load(const std::string& file);

    Utils::VariantMap config_;
};

