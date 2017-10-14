#pragma once

#include "Variant.h"

class ConfigManager
{
public:
    enum Key
    {
        GamePort,
        AdminPort
    };
public:
    ConfigManager();
    ~ConfigManager();

    static ConfigManager Instance;

    VariantMap config_;
};

