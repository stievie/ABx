#pragma once

#include "Actor.h"
#include "Chat.h"
#include "Script.h"
#include "AiLoader.h"
#include "AiCharacter.h"
#include "TriggerComp.h"

namespace Game {

class Npc;
class Map;

class Npc final : public Actor
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionOnTrigger = 1 << 1,
        FunctionOnLeftArea = 1 << 2,
    };

    friend class AI::AiCharacter;
    /// This NPC exists only on the server, i.e. is not spawned on the client, e.g. a trigger box.
    bool serverOnly_;
    std::string name_;
    uint32_t level_;
    uint32_t modelIndex_;
    AB::Entities::CharacterSex sex_;
    /// Group is like a party and they need unique IDs.
    /// If this NPC belongs to a party with players this must be the PartyID.
    uint32_t groupId_;
    std::string behaviorTree_;
    std::shared_ptr<Script> script_;
    std::shared_ptr<AI::AiCharacter> aiCharacter_;
    std::shared_ptr<ai::AI> ai_;
    uint32_t functions_;
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
protected:
    kaguya::State luaState_;
    bool luaInitialized_;
    void InitializeLua();
    void OnArrived() override;
public:
    static void RegisterLua(kaguya::State& state);

    Npc();
    ~Npc();
    // non-copyable
    Npc(const Npc&) = delete;
    Npc& operator=(const Npc&) = delete;

    bool LoadScript(const std::string& fileName);
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::ObjectTypeNpc;
    }

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;
    std::shared_ptr<ai::AI> GetAi();
    void Shutdown();
    bool HealSelf();

    std::string GetName() const override { return name_; }
    void SetName(const std::string& name) { name_ = name; }
    uint32_t GetLevel() const override { return level_; }
    void SetLevel(uint32_t value) { level_ = value; }
    uint32_t GetModelIndex() const final override
    {
        return modelIndex_;
    }
    AB::Entities::CharacterSex GetSex() const final override
    {
        return sex_;
    }
    uint32_t GetGroupId() const final override
    {
        return groupId_;
    }
    void SetGroupId(uint32_t value);
    bool SetBehaviour(const std::string& name);
    const std::string& GetBehaviour() const { return behaviorTree_; }
    float GetAggro(const Actor* other);
    bool IsServerOnly() const { return serverOnly_; }
    void SetServerOnly(bool value) { serverOnly_ = value; }

    void WriteSpawnData(Net::NetworkMessage& msg) override;

    void Say(ChatType channel, const std::string& message);

    void OnSelected(Actor* selector) override;
    void OnClicked(Actor* selector) override;
    void OnCollide(GameObject* other) override;
    void OnTrigger(GameObject* other) override;
    void OnLeftArea(GameObject* other) override;
    void OnEndUseSkill(Skill* skill) override;
    void OnStartUseSkill(Skill* skill) override;

    bool OnAttack(Actor* target) override;
    bool OnAttacked(Actor* source, DamageType type, int32_t damage) override;
    bool OnGettingAttacked(Actor* source) override;
    bool OnUseSkill(Actor* target, Skill* skill) override;
    bool OnSkillTargeted(Actor* source, Skill* skill) override;
    bool OnInterruptingAttack() override;
    bool OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill) override;
    void OnInterruptedAttack() override;
    void OnInterruptedSkill(Skill* skill) override;
    void OnKnockedDown(uint32_t time) override;
    void OnHealed(int hp) override;
    void OnDied() override;
    void OnResurrected(int health, int energy) override;
};

}
