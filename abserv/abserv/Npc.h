#pragma once

#include "Actor.h"
#include "Chat.h"
#include "Script.h"
#include "AiLoader.h"
#include "AiCharacter.h"
#include "TriggerComp.h"

namespace Game {

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
        FunctionOnGetQuote = 1 << 3
    };

    friend class AI::AiCharacter;
    /// This NPC exists only on the server, i.e. is not spawned on the client, e.g. a trigger box.
    bool serverOnly_{ false };
    uint32_t level_{ 1 };
    uint32_t itemIndex_{ 0 };
    AB::Entities::CharacterSex sex_{ AB::Entities::CharacterSexUnknown };
    /// Group is like a party and they need unique IDs.
    /// If this NPC belongs to a party with players this must be the PartyID.
    uint32_t groupId_;
    std::string behaviorTree_;
    std::shared_ptr<Script> script_;
    std::shared_ptr<AI::AiCharacter> aiCharacter_;
    std::shared_ptr<ai::AI> ai_;
    uint32_t functions_{ FunctionNone };
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
    kaguya::State luaState_;
    bool luaInitialized_;
    void InitializeLua();
    std::string GetQuote(int index);
private:
    // Events
    void OnArrived();
    void OnInterruptedAttack();
    void OnInterruptedSkill(Skill* skill);
    void OnKnockedDown(uint32_t time);
    void OnHealed(int hp);
    void OnResurrected(int health, int energy);
    void OnClicked(Actor* selector);
    void OnCollide(GameObject* other);
    void OnSelected(Actor* selector);
    void OnTrigger(GameObject* other);
    void OnLeftArea(GameObject* other);
    void OnDied();
    void OnEndUseSkill(Skill* skill);
    void OnStartUseSkill(Skill* skill);
    void OnAttack(Actor* target, bool& canAttack);
    void OnGettingAttacked(Actor* source, bool& canGetAttacked);
    void OnAttacked(Actor* source, DamageType type, int32_t damage, bool& canGetAttacked);
    void OnInterruptingAttack(bool& success);
    void OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill, bool& success);
    void OnUseSkill(Actor* target, Skill* skill, bool& success);
    void OnSkillTargeted(Actor* source, Skill* skill, bool& succcess);
public:
    static void RegisterLua(kaguya::State& state);

    Npc();
    ~Npc() final;
    // non-copyable
    Npc(const Npc&) = delete;
    Npc& operator=(const Npc&) = delete;

    bool LoadScript(const std::string& fileName);
    AB::GameProtocol::GameObjectType GetType() const final override
    {
        return AB::GameProtocol::ObjectTypeNpc;
    }

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;
    std::shared_ptr<ai::AI> GetAi();
    void Shutdown();

    uint32_t GetLevel() const override { return level_; }
    bool CanAttack() const override { return true; }
    bool CanUseSkill() const override { return true; }
    void SetLevel(uint32_t value) override;
    uint32_t GetItemIndex() const final override
    {
        return itemIndex_;
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
    bool SayQuote(ChatType channel, int index);
    /// Shooting a projectile without having a weapon that can spawn projectiles
    void ShootAt(const std::string& itemUuid, Actor* target);
};

}
