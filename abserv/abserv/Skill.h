#pragma once

#include <memory>
#include <string>
#pragma warning(push)
#pragma warning(disable: 4702 4127 4244)
#include <kaguya/kaguya.hpp>
#pragma warning(pop)

namespace Game {

class Creature;

class Skill : public std::enable_shared_from_this<Skill>
{
private:
    kaguya::State luaState_;
    int64_t startUse_;
    void InitializeLua();
    Creature* source_;
    Creature* target_;
public:
    static void RegisterLua(kaguya::State& state);

    explicit Skill(uint32_t id) :
        id_(id),
        startUse_(0)
    {
        InitializeLua();
    }
    ~Skill() = default;

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);

    bool StartUse(Creature* source, Creature* target);
    void CancelUse();

    bool Using() const { return startUse_ != 0; }

    uint32_t id_;

    std::string name_;
    uint32_t costEnergy_;
    uint32_t costAdrenaline_;
    uint32_t activation_;
    uint32_t recharge_;
    uint32_t overcast_;
};

}
