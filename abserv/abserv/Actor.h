/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "AttackComp.h"
#include "AutoRunComp.h"
#include "Effect.h"
#include "EffectsComp.h"
#include "GameObject.h"
#include "InputComp.h"
#include "InputQueue.h"
#include "InventoryComp.h"
#include "MoveComp.h"
#include "ResourceComp.h"
#include "SkillBar.h"
#include "SkillsComp.h"
#include <abscommon/UuidUtils.h>
#include <AB/ProtocolCodes.h>

namespace Game {

namespace Components {
class CollisionComp;
class DamageComp;
class HealComp;
class ProgressComp;
class SelectionComp;
}

enum class TargetClass
{
    All,
    Friend,
    Foe
};

inline constexpr sa::event_t EVENT_ON_ARRIVED = sa::StringHash("OnArrived");
inline constexpr sa::event_t EVENT_ON_INTERRUPTEDATTACK = sa::StringHash("OnInterruptedAttack");
inline constexpr sa::event_t EVENT_ON_INTERRUPTEDSKILL = sa::StringHash("OnInterruptedSkill");
inline constexpr sa::event_t EVENT_ON_KNOCKED_DOWN = sa::StringHash("OnKnockedDown");
inline constexpr sa::event_t EVENT_ON_HEALED = sa::StringHash("OnHealed");
inline constexpr sa::event_t EVENT_ON_DIED = sa::StringHash("OnDied");                                    // Arguments: Actor* (who died), Actor* (who killed the poor victim)
inline constexpr sa::event_t EVENT_ON_RESURRECTED = sa::StringHash("OnResurrected");
inline constexpr sa::event_t EVENT_ON_PINGOBJECT = sa::StringHash("OnPingObject");
inline constexpr sa::event_t EVENT_ON_INVENTORYFULL = sa::StringHash("OnInventoryFull");
inline constexpr sa::event_t EVENT_ON_CHESTFULL = sa::StringHash("OnChestFull");
inline constexpr sa::event_t EVENT_ON_ATTACK = sa::StringHash("OnAttack");
inline constexpr sa::event_t EVENT_ON_ATTACKED = sa::StringHash("OnAttacked");
inline constexpr sa::event_t EVENT_ON_GETTING_ATTACKED = sa::StringHash("OnGettingAttacked");
inline constexpr sa::event_t EVENT_ON_USESKILL = sa::StringHash("OnUseSkill");
inline constexpr sa::event_t EVENT_ON_SKILLTARGETED = sa::StringHash("OnSkillTargeted");
inline constexpr sa::event_t EVENT_ON_GET_CRITICAL_HIT = sa::StringHash("OnGetCriticalHit");
inline constexpr sa::event_t EVENT_ON_ENDUSESKILL = sa::StringHash("OnEndUseSkill");
inline constexpr sa::event_t EVENT_ON_STARTUSESKILL = sa::StringHash("OnStartUseSkill");
inline constexpr sa::event_t EVENT_ON_HANDLECOMMAND = sa::StringHash("OnHandleCommand");
inline constexpr sa::event_t EVENT_ON_INTERRUPTING_ATTACK = sa::StringHash("OnInterruptingAttack");
inline constexpr sa::event_t EVENT_ON_INTERRUPTING_SKILL = sa::StringHash("OnInterruptingSkill");
inline constexpr sa::event_t EVENT_ON_KNOCKING_DOWN = sa::StringHash("OnKnockingDown");
inline constexpr sa::event_t EVENT_ON_HEALING = sa::StringHash("OnHealing");
inline constexpr sa::event_t EVENT_ON_STUCK = sa::StringHash("OnStuck");
inline constexpr sa::event_t EVENT_ON_INCMORALE = sa::StringHash("OnIncMorale");
inline constexpr sa::event_t EVENT_ON_DECMORALE = sa::StringHash("OnDecMorale");
inline constexpr sa::event_t EVENT_ON_KILLEDFOE = sa::StringHash("OnKilledFoe");

class AreaOfEffect;
class Group;

/// Player, NPC, Monster some such
class Actor : public GameObject
{
    friend class Components::MoveComp;              // Needed for accessing octand
private:
    void _LuaGotoPosition(const Math::StdVector3& pos);
    void _LuaSetHomePos(const Math::StdVector3& pos);
    void _LuaHeadTo(const Math::StdVector3& pos);
    Math::StdVector3 _LuaGetHomePos();
    void _LuaFollowObject(GameObject* object);
    void _LuaAddEffect(Actor* source, uint32_t index, uint32_t time);
    void _LuaRemoveEffect(uint32_t index);
    Effect* _LuaGetLastEffect(AB::Entities::EffectCategory category);
    GameObject* _LuaGetSelectedObject();
    void _LuaSetSelectedObject(GameObject* object);
    std::vector<Actor*> _LuaGetActorsInRange(Ranges range);
    std::vector<Actor*> _LuaGetAlliesInRange(Ranges range);
    std::vector<Actor*> _LuaGetEnemiesInRange(Ranges range);
    AreaOfEffect* _LuaAddAOE(const std::string& script,
        uint32_t index,
        const Math::StdVector3& pos);
    bool _LuaHasEffect(uint32_t index);
    /// Get lower 16 bits of the group mask
    uint32_t GetFriendMask() const { return groupMask_ & 0xffff; }
    /// Get upper 16 bits of the group mask
    uint32_t GetFoeMask() const { return groupMask_ >> 16; }
protected:
    Math::Vector3 homePos_;
    ea::weak_ptr<Actor> killedBy_;
    int morale_{ 0 };
    /// Group/Party ID
    uint32_t groupId_{ 0 };
private:
    // Events
    void OnEndUseSkill(Skill* skill);
    void OnStartUseSkill(Skill* skill);
public:
    static void RegisterLua(kaguya::State& state);

    Actor();
    ~Actor() override;

    /// Loading is done initialize properties
    virtual void Initialize();
    bool SetSpawnPoint(const std::string& group);
    void SetHomePos(const Math::Vector3& pos)
    {
        homePos_ = pos;
    }
    const Math::Vector3& GetHomePos() const { return homePos_; }
    bool GotoHomePos();
    template<typename Func>
    void VisitEnemiesInRange(Ranges range, Func&& func) const
    {
        VisitInRange(range, [&](const GameObject& o)
        {
            if (o.IsPlayerOrNpcType())
            {
                const auto& actor = To<Actor>(o);
                if (this->IsEnemy(&actor))
                    return func(actor);
            }
            return Iteration::Continue;
        });
    }
    size_t GetEnemyCountInRange(Ranges range) const;
    /// Ally is always without self
    template<typename Func>
    void VisitAlliesInRange(Ranges range, Func&& func) const
    {
        VisitInRange(range, [&](const GameObject& o)
        {
            if (o.IsPlayerOrNpcType())
            {
                const auto& actor = To<Actor>(o);
                if (this->IsAlly(&actor))
                    return func(actor);
            }
            return Iteration::Continue;
        });
    }
    size_t GetAllyCountInRange(Ranges range) const;
    Actor* GetClosestEnemy(bool undestroyable, bool unselectable);
    Actor* GetClosestAlly(bool undestroyable, bool unselectable);
    Group* GetGroup() const;
    Item* GetWeapon() const;
    Actor* GetKiller() const
    {
        if (auto k = killedBy_.lock())
            return k.get();
        return nullptr;
    }

    void HeadTo(const Math::Vector3& pos);
    void FaceObject(GameObject* object);
    /// Move speed: 1 = normal speed
    float GetSpeed() const { return moveComp_->GetSpeedFactor(); }
    void SetSpeed(float value) { moveComp_->SetSpeedFactor(value); }
    void AddSpeed(float value) { moveComp_->AddSpeed(value); }
    bool IsMoving() const { return moveComp_->IsMoving(); }
    void SetState(AB::GameProtocol::CreatureState state) override;
    /// Attack hit
    bool IsHitting() const { return attackComp_->IsHitting(); }
    bool IsUndestroyable() const { return undestroyable_; }
    void SetUndestroyable(bool value) { undestroyable_ = value; }
    bool IsInWeaponRange(Actor* actor) const;
    /// Get effect of armor. Armor is influenced by the equipment and effects
    /// Damage multiplier.
    float GetArmorEffect(DamageType damageType, DamagePos pos, float penetration);
    /// Attack speed in ms. One attack cycle (start hit -> hit -> apply damage) takes this ms to complete.
    uint32_t GetAttackSpeed();
    /// Get increased attack speed relative to normal weapon attack speed. > 1 = faster, < 1 slower
    float GetAttackSpeedIncrease(uint32_t speed);
    DamageType GetAttackDamageType();
    int32_t GetAttackDamage(bool critical);
    float GetArmorPenetration();
    /// Get chance for a critical hit. Value between 0..1
    float GetCriticalChance(const Actor* other);
    DamagePos GetDamagePos() const;
    int GetResource(Components::ResourceType type) const { return resourceComp_->GetValue(type); }
    void SetResource(Components::ResourceType type, Components::SetValueType t, int value);
    /// Steal life from this actor. The source must add the returned value to its life.
    int DrainLife(Actor* source, uint32_t index, int value);
    /// Steal energy from this actor. The source must add the returned value to its energy.
    int DrainEnergy(int value);
    int AddEnergy(int value);
    void SetHealthRegen(int value);
    int GetMorale() const;
    bool IncreaseMorale();
    bool DecreaseMorale();

    bool InterruptAttack();
    bool InterruptSkill(AB::Entities::SkillType type);
    /// Interrupt everything
    bool Interrupt();

    virtual bool CanAttack() const { return false; }
    virtual bool CanUseSkill() const { return false; }
    virtual uint32_t GetLevel() const { return 0; }
    std::string GetClassLevel() const;
    virtual void SetLevel(uint32_t) { }
    virtual void AddXp(int) { }
    virtual uint32_t GetXp() const { return 0; }
    /// Get the number of attribute points according the level
    virtual uint32_t GetAttributePoints() const;
    virtual void AddSkillPoint() { }
    virtual uint32_t GetSkillPoints() const { return 0; }
    virtual void AdvanceLevel();
    virtual AB::Entities::CharacterSex GetSex() const
    {
        return AB::Entities::CharacterSex::Unknown;
    }
    virtual uint32_t GetItemIndex() const
    {
        return 0;
    }
    uint32_t GetGroupId() const { return groupId_; }
    void SetGroupId(uint32_t value) { groupId_ = value; }
    virtual size_t GetGroupPos() { return 0u; }
    AB::Entities::ProfessionIndex GetProfIndex() const
    {
        return static_cast<AB::Entities::ProfessionIndex>(skills_->prof1_.index);
    }
    AB::Entities::ProfessionIndex GetProf2Index() const
    {
        return static_cast<AB::Entities::ProfessionIndex>(skills_->prof2_.index);
    }
    /// 0 Based
    Skill* GetSkill(uint32_t index)
    {
        if (index < PLAYER_MAX_SKILLS)
            return (*skills_)[index];
        return nullptr;
    }
    SkillBar* GetSkillBar()
    {
        return skills_.get();
    }
    Skill* GetCurrentSkill() const;
    bool IsUsingSkillOfType(AB::Entities::SkillType type, int32_t minActivationTime) const;
    bool SetEquipment(const std::string& ciUuid);
    bool SetInventory(const std::string& ciUuid);
    bool SetChest(const std::string& ciUuid);
    virtual const std::string& GetPlayerUuid() const;
    virtual const std::string& GetAccountUuid() const;

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    virtual bool Die(Actor* killer);
    virtual bool Resurrect(int precentHealth, int percentEnergy);
    bool IsDead() const { return stateComp_.IsDead(); }
    unsigned GetDeaths() const;
    bool IsKnockedDown() const { return stateComp_.IsKnockedDown(); }
    /// Returns true when the actor can't do anything
    bool IsImmobilized() const { return stateComp_.IsDead() || stateComp_.IsKnockedDown(); }
    /// Knock the Actor down caused by source. If time = 0 DEFAULT_KNOCKDOWN_TIME is used.
    bool KnockDown(Actor* source, uint32_t time);
    int Healing(Actor* source, uint32_t index, int value);
    int Damage(Actor* source, uint32_t index, DamageType type, int value);
    bool IsEnemy(const Actor* other) const;
    bool IsAlly(const Actor* other) const;
    void AddFriendFoe(uint32_t frnd, uint32_t foe);
    void RemoveFriendFoe(uint32_t frnd, uint32_t foe);
    void AddInput(InputType type, Utils::VariantMap&& data)
    {
        inputComp_->Add(type, std::move(data));
    }
    void AddInput(InputType type)
    {
        inputComp_->Add(type);
    }
    // Get the effective attribute rank. This is attributes from Skills+Equipment+Effects
    uint32_t GetAttributeRank(Attribute index);

    GameObject* GetSelectedObject() const;
    uint32_t GetSelectedObjectId() const;
    bool SelectedObject(GameObject* object);
    bool SelectedObjectById(uint32_t id);
    void GotoPosition(const Math::Vector3& pos);
    bool FollowObject(GameObject* object, bool ping);
    bool FollowObjectById(uint32_t objectId, bool ping);
    bool UseSkill(int index, bool ping);
    bool Attack(Actor* target, bool ping);
    bool AttackById(uint32_t targetId, bool ping);
    bool IsAttackingActor(const Actor* target) const;
    /// Cancel attack, use skill, follow
    void CancelAction();
    void CancelAll();

    virtual bool AddToInventory(uint32_t itemId);
    void DropRandomItem();

    ea::unique_ptr<SkillBar> skills_;

    ea::unique_ptr<Components::ResourceComp> resourceComp_;
    ea::unique_ptr<Components::AttackComp> attackComp_;
    ea::unique_ptr<Components::SkillsComp> skillsComp_;
    ea::unique_ptr<Components::InputComp> inputComp_;
    ea::unique_ptr<Components::DamageComp> damageComp_;
    ea::unique_ptr<Components::HealComp> healComp_;
    ea::unique_ptr<Components::AutoRunComp> autorunComp_;
    ea::unique_ptr<Components::ProgressComp> progressComp_;
    ea::unique_ptr<Components::EffectsComp> effectsComp_;
    ea::unique_ptr<Components::InventoryComp> inventoryComp_;
    ea::unique_ptr<Components::MoveComp> moveComp_;
    ea::unique_ptr<Components::CollisionComp> collisionComp_;
    ea::unique_ptr<Components::SelectionComp> selectionComp_;

    bool undestroyable_{ false };
    /// Friend foe identification. Upper 16 bit is foe mask, lower 16 bit friend mask.
    /// This gives us 3 types of relation: (1) friend, (2) foe and (3) neutral
    /// and (4) in theory, both but that would be silly.
    uint32_t groupMask_{ 0 };

    bool Serialize(sa::PropWriteStream& stream) override;
    void WriteSpawnData(Net::NetworkMessage& msg) override;
};

template <>
inline bool Is<Actor>(const GameObject& obj)
{
    return obj.GetType() >= AB::GameProtocol::GameObjectType::Projectile;
}

inline bool TargetClassMatches(const Actor& actor, TargetClass _class, const Actor& target)
{
    return ((_class == TargetClass::All) ||
        (_class == TargetClass::Foe && actor.IsEnemy(&target)) ||
        (_class == TargetClass::Friend && actor.IsAlly(&target)));
}

inline void GetSkillRecharge(const Actor& actor, Skill* skill, uint32_t& recharge)
{
    actor.inventoryComp_->GetSkillRecharge(skill, recharge);
    actor.effectsComp_->GetSkillRecharge(skill, recharge);
    actor.skillsComp_->GetSkillRecharge(skill, recharge);
}

inline void GetSkillCost(const Actor& actor, Skill* skill,
    int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp)
{
    actor.skillsComp_->GetSkillCost(skill, activation, energy, adrenaline, overcast, hp);

    actor.inventoryComp_->GetSkillCost(skill, activation, energy, adrenaline, overcast, hp);
    actor.effectsComp_->GetSkillCost(skill, activation, energy, adrenaline, overcast, hp);
}

inline void GetResources(const Actor& actor, int& maxHealth, int& maxEnergy)
{
    // Attributtes first
    actor.skillsComp_->GetResources(maxHealth, maxEnergy);
    // Runes etc.
    actor.inventoryComp_->GetResources(maxHealth, maxEnergy);
    // Effects influencing
    actor.effectsComp_->GetResources(maxHealth, maxEnergy);
}

}
