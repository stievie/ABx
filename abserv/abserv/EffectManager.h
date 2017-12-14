#pragma once

namespace Game {

class Effect;

class EffectManager
{
public:
    EffectManager() = default;
    ~EffectManager() = default;

    std::map<uint32_t, std::string> effects_;
    std::map<std::string, uint32_t> effectNames_;

    std::unique_ptr<Effect> Get(uint32_t id);
    std::unique_ptr<Effect> Get(const std::string& name);
    uint32_t GetEffectId(const std::string& name);
public:
    static EffectManager Instance;

};

}
