#pragma once

#include "Actor.h"
#include "Chat.h"
#include "Script.h"
#include "AiLoader.h"
#include "TriggerComp.h"
#include "AiComp.h"
#include "WanderComp.h"

namespace Game {

class Map;
class Crowd;

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

    /// This NPC exists only on the server, i.e. is not spawned on the client, e.g. a trigger box.
    bool serverOnly_{ false };
    uint32_t level_{ 1 };
    uint32_t itemIndex_{ 0 };
    AB::Entities::CharacterSex sex_{ AB::Entities::CharacterSexUnknown };
    std::shared_ptr<Script> script_;
    uint32_t functions_{ FunctionNone };
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
    kaguya::State luaState_;
    bool luaInitialized_;
    void InitializeLua();
    std::string GetQuote(int index);
    void _LuaAddWanderPoint(const Math::STLVector3& point);
    void _LuaAddWanderPoints(const std::vector<Math::STLVector3>& points);
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

    uint32_t GetLevel() const override { return level_; }
    bool CanAttack() const override { return true; }
    bool CanUseSkill() const override { return true; }
    void SetLevel(uint32_t value) override;
    uint32_t GetItemIndex() const final override
    {
        return itemIndex_;
    }
    AB::Entities::CharacterSex GetSex() const override
    {
        return sex_;
    }
    bool SetBehavior(const std::string& name);
    float GetAggro(const Actor* other);
    int GetBestSkillIndex(SkillEffect effect, SkillTarget target,
        AB::Entities::SkillType interrupts = AB::Entities::SkillTypeAll,
        const Actor* targetActor = nullptr);
    bool IsServerOnly() const { return serverOnly_; }
    void SetServerOnly(bool value) { serverOnly_ = value; }
    bool IsWander() const;
    void SetWander(bool value);

    void WriteSpawnData(Net::NetworkMessage& msg) override;

    void Say(ChatType channel, const std::string& message);
    bool SayQuote(ChatType channel, int index);
    /// Shooting a projectile without having a weapon that can spawn projectiles
    void ShootAt(const std::string& itemUuid, Actor* target);

    std::unique_ptr<Components::AiComp> aiComp_;
    std::unique_ptr<Components::WanderComp> wanderComp_;
};

template <>
inline bool Is<Npc>(const GameObject& obj)
{
    return obj.GetType() == AB::GameProtocol::ObjectTypeNpc;
}

}
