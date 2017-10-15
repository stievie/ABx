#include "stdafx.h"
#include "ConfigManager.h"

ConfigManager ConfigManager::Instance;

ConfigManager::ConfigManager()
{
    config_[Key::GamePort] = 1337;
    config_[Key::LoginPort] = 1338;
}

ConfigManager::~ConfigManager()
{
}
