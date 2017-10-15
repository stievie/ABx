#pragma once

#include "Variant.h"

class ConfigManager
{
public:
    enum Key
    {
        GamePort,
        AdminPort,
        LoginPort
    };
public:
    ConfigManager();
    ~ConfigManager();

    static ConfigManager Instance;

    VariantMap config_;
};

