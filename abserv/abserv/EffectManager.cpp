#include "stdafx.h"
#include "EffectManager.h"
#include "Effect.h"
#include "ConfigManager.h"

namespace Game {

EffectManager EffectManager::Instance;

std::unique_ptr<Effect> EffectManager::Get(uint32_t id)
{
    auto it = effects_.find(id);
    if (it == effects_.end())
        return std::unique_ptr<Effect>();

    std::unique_ptr<Effect> result = std::make_unique<Effect>(id);
    if (result->LoadScript(ConfigManager::Instance.GetDataFile((*it).second)))
        return result;

    return std::unique_ptr<Effect>();
}

std::unique_ptr<Effect> EffectManager::Get(const std::string& name)
{
    auto it = effectNames_.find(name);
    if (it == effectNames_.end())
        return std::unique_ptr<Effect>();

    uint32_t id = (*it).second;
    return Get(id);
}

uint32_t EffectManager::GetEffectId(const std::string & name)
{
    auto it = effectNames_.find(name);
    if (it == effectNames_.end())
        return 0;

    return (*it).second;
}

}
