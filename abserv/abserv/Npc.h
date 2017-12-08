#pragma once

#include "Creature.h"
#pragma warning(push)
#pragma warning(disable: 4702 4127)
#include <kaguya/kaguya.hpp>
#pragma warning(pop)

namespace Game {

class Npc final : public Creature
{
private:
    kaguya::State luaState_;
    void InitializeLua();
    std::string name_;
    uint32_t level_;
public:
    static void RegisterLua(kaguya::State& state);

    Npc();
    ~Npc() override;
    // non-copyable
    Npc(const Npc&) = delete;
    Npc& operator=(const Npc&) = delete;

    bool LoadScript(const std::string& fileName);
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::ObjectTypeNpc;
    }

    void Update(uint32_t timeElapsed) override;

    std::string GetName() const override { return name_; }
    uint32_t GetLevel() const override { return level_; }

};

}
