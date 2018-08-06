#pragma once

#include "Creature.h"

namespace Game {

class Npc final : public Creature
{
private:
    kaguya::State luaState_;
    void InitializeLua();
    std::string name_;
    uint32_t level_;
    uint32_t modelIndex_;
    AB::Entities::CharacterSex sex_;

    void _LuaSetPosition(float x, float y, float z);
    void _LuaSetRotation(float y);
    void _LuaSetScale(float x, float y, float z);
    std::vector<float> _LuaGetPosition() const;
    float _LuaGetRotation() const;
    std::vector<float> _LuaGetScale() const;
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

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    std::string GetName() const override { return name_; }
    uint32_t GetLevel() const override { return level_; }
    uint32_t GetModelIndex() const final override
    {
        return modelIndex_;
    }
    AB::Entities::CharacterSex GetSex() const final override
    {
        return sex_;
    }

    void OnSelected(Creature* selector) override final;
};

}
