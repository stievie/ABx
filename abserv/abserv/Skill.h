#pragma once

#include <memory>
#include <string>
#pragma warning(push)
#pragma warning(disable: 4702 4127 4244)
#include <kaguya/kaguya.hpp>
#pragma warning(pop)
#include "Utils.h"

namespace Game {

class Creature;

class Skill : public std::enable_shared_from_this<Skill>
{
private:
    kaguya::State luaState_;
    int64_t startUse_;
    int64_t recharged_;
    void InitializeLua();
    Creature* source_;
    Creature* target_;
public:
    static void RegisterLua(kaguya::State& state);

    explicit Skill(uint32_t id) :
        id_(id),
        startUse_(0),
        recharged_(0)
    {
        InitializeLua();
    }
    ~Skill() = default;

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);

    bool StartUse(Creature* source, Creature* target);
    void CancelUse();

    bool IsUsing() const { return startUse_ != 0; }
    bool IsRecharged() const { return recharged_ <= Utils::AbTick(); }

    uint32_t id_;

    std::string name_;
    uint32_t energy_;
    uint32_t adrenaline_;
    uint32_t activation_;
    uint32_t recharge_;
    uint32_t overcast_;
};

}
