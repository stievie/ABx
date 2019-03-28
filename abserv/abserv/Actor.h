#pragma once

#include "GameObject.h"
#include "Effect.h"
#include "SkillBar.h"
#include "InputQueue.h"
#include <AB/ProtocolCodes.h>
#include "MoveComp.h"
#include "AutoRunComp.h"
#include "ResourceComp.h"
#include "CollisionComp.h"
#include "AttackComp.h"
#include "EffectsComp.h"
#include "EquipComp.h"
#include "SkillsComp.h"
#include "InputComp.h"
#include "DamageComp.h"
#include "HealComp.h"
#include "ProgressComp.h"
#include "UuidUtils.h"

namespace Game {

/// Player, NPC, Monster some such
class Actor : public GameObject
{
    friend class Components::MoveComp;
    friend class Components::AutoRunComp;
    friend class Components::CollisionComp;
    friend class Components::ResourceComp;
    friend class Components::AttackComp;
    friend class Components::EffectsComp;
    friend class Components::EquipComp;
    friend class Components::SkillsComp;
    friend class Components::InputComp;
    friend class Components::DamageComp;
    friend class Components::HealComp;
    friend class Components::ProgressComp;
private:
    void _LuaGotoPosition(float x, float y, float z);
    int _LuaGetState();
    void _LuaSetState(int state);
    void _LuaSetHomePos(float x, float y, float z);
    void _LuaHeadTo(float x, float y, float z);
    std::vector<float> _LuaGetHomePos();
    void _LuaFollowObject(GameObject* object);
    void _LuaAddEffect(Actor* source, uint32_t index);
    void _LuaRemoveEffect(uint32_t index);
    Effect* _LuaGetLastEffect(AB::Entities::EffectCategory category);
    GameObject* _LuaGetSelectedObject();
    void _LuaSetSelectedObject(GameObject* object);
protected:
    std::vector<Math::Vector3> wayPoints_;

    Math::Vector3 homePos_;
    virtual void HandleCommand(AB::GameProtocol::CommandTypes type,
        const std::string& command, Net::NetworkMessage& message)
    {
        AB_UNUSED(type);
        AB_UNUSED(command);
        AB_UNUSED(message);
    }
    virtual void OnArrived() {}
public:
    static void RegisterLua(kaguya::State& state);

    Actor();

    /// Loading is done initialize properties
    virtual void Initialize();
    bool SetSpawnPoint(const std::string& group);
    void SetHomePos(const Math::Vector3& pos)
    {
        homePos_ = pos;
    }
    const Math::Vector3& GetHomePos() const { return homePos_; }
    bool GotoHomePos();
    bool IsInRange(Ranges range, Actor* actor)
    {
        if (!actor)
            return false;
        if (range == Ranges::Map)
            return true;
        // Don't calculate the distance now, but use previously calculated values.
        for (const auto& o : ranges_[range])
        {
            if (auto so = o.lock())
            {
                if (so->id_ == actor->id_)
                    return true;
            }
        }
        return false;
    }
    std::vector<Actor*> GetEnemiesInRange(Ranges range);
    size_t GetEnemyCountInRange(Ranges range);
    std::vector<Actor*> GetAlliesInRange(Ranges range);
    size_t GetAllyCountInRange(Ranges range);
    Item* GetWeapon() const;
    virtual void OnEndUseSkill(Skill* skill);
    virtual void OnStartUseSkill(Skill* skill);

    void HeadTo(const Math::Vector3& pos);
    void FaceObject(GameObject* object);
    /// Move speed: 1 = normal speed
    float GetSpeed() const { return moveComp_.GetSpeedFactor(); }
    void SetSpeed(float value) { moveComp_.SetSpeedFactor(value); }
    void AddSpeed(float value) { moveComp_.AddSpeed(value); }
    bool IsMoving() const { return moveComp_.IsMoving(); }
    bool IsAttacking() const { return attackComp_.IsAttacking(); }
    bool IsUndestroyable() const { return undestroyable_; }
    void SetUndestroyable(bool value) { undestroyable_ = value; }
    bool IsInWeaponRange(Actor* actor) const;
    /// Get effect of armor. Armor is influenced by the equipment and effects
    /// Damage multiplier
    float GetArmorEffect(DamageType damageType, DamagePos pos, float penetration);
    uint32_t GetAttackSpeed();
    DamageType GetAttackDamageType();
    int32_t GetAttackDamage(bool critical);
    float GetArmorPenetration();
    /// Get chance for a critical hit. Value between 0..1
    float GetCriticalChance(Actor* other);
    DamagePos GetDamagePos() const { return damageComp_.GetDamagePos(); }
    int GetResource(Components::ResourceType type) const { return resourceComp_.GetValue(type); }
    void SetResource(Components::ResourceType type, Components::SetValueType t, int value);
    /// This Actor is attacking the target
    virtual bool OnAttack(Actor* target);
    /// This Actor was attacked by source
    virtual bool OnAttacked(Actor* source, DamageType type, int32_t damage);
    /// This Actor is going to bee attacked by source. Happens before OnAttacked.
    virtual bool OnGettingAttacked(Actor* source);
    /// This actor is using a skill on target
    virtual bool OnUseSkill(Actor* target, Skill* skill);
    /// This Actor is targeted for a skill by source
    virtual bool OnSkillTargeted(Actor* source, Skill* skill);
    virtual bool OnGetCriticalHit(Actor* source);
    /// Adds damage to this actor, skill or effect index my be 0.
    void ApplyDamage(Actor* source, uint32_t index, DamageType type, int value, float penetration);
    /// Steal life from this actor. The source must add the returned value to its life.
    int DrainLife(Actor* source, uint32_t index, int value);
    /// Steal energy from this actor. The source must add the returned value to its energy.
    int DrainEnergy(int value);

    virtual bool OnInterruptingAttack();
    virtual bool OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill);
    bool InterruptAttack();
    bool InterruptSkill(AB::Entities::SkillType type);
    /// Interrupt everything
    bool Interrupt();
    virtual void OnInterruptedAttack() { }
    virtual void OnInterruptedSkill(Skill*) { }
    virtual void OnKnockedDown(uint32_t) { }
    virtual void OnHealed(int) { }
    virtual void OnDied();
    virtual void OnResurrected(int /* health */, int /* energy */) { }

    virtual uint32_t GetLevel() const { return 0; }
    virtual void SetLevel(uint32_t) { }
    virtual void AddXp(int) { }
    virtual uint32_t GetXp() const { return 0; }
    virtual void AddSkillPoint() { }
    virtual uint32_t GetSkillPoints() const { return 0; }
    virtual void AdvanceLevel() { }
    virtual AB::Entities::CharacterSex GetSex() const
    {
        return AB::Entities::CharacterSexUnknown;
    }
    virtual uint32_t GetModelIndex() const
    {
        return 0;
    }
    virtual uint32_t GetGroupId() const { return 0u; }
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
    bool SetEquipment(const std::string& ciUuid);

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    virtual bool Die();
    virtual bool Resurrect(int precentHealth, int percentEnergy);
    bool IsDead() const { return stateComp_.IsDead(); }
    bool IsKnockedDown() const { return stateComp_.IsKnockedDown(); }
    /// Returns true when the actor can't do anything
    bool IsImmobilized() const { return stateComp_.IsDead() || stateComp_.IsKnockedDown(); }
    bool KnockDown(Actor* source, uint32_t time);
    int Healing(Actor* source, uint32_t index, int value);
    bool IsEnemy(Actor* other);
    inline void AddInput(InputType type, const Utils::VariantMap& data)
    {
        inputComp_.Add(type, data);
    }
    inline void AddInput(InputType type)
    {
        inputComp_.Add(type);
    }
    uint32_t GetAttributeValue(uint32_t index);

    std::weak_ptr<GameObject> selectedObject_;
    std::weak_ptr<GameObject> followedObject_;
    uint32_t GetSelectedObjectId() const
    {
        if (auto sel = selectedObject_.lock())
            return sel->GetId();
        return 0;
    }
    std::shared_ptr<GameObject> GetSelectedObject() const
    {
        return selectedObject_.lock();
    }
    void SetSelectedObject(std::shared_ptr<GameObject> object);
    void GotoPosition(const Math::Vector3& pos);
    void FollowObject(std::shared_ptr<GameObject> object);
    void FollowObject(uint32_t objectId);
    void UseSkill(uint32_t index);
    /// Cancel attack, use skill, follow
    void CancelAction();

    std::unique_ptr<SkillBar> skills_;

    Components::MoveComp moveComp_;
    Components::AutoRunComp autorunComp_;
    Components::ResourceComp resourceComp_;
    Components::AttackComp attackComp_;
    Components::EffectsComp effectsComp_;
    Components::EquipComp equipComp_;
    Components::SkillsComp skillsComp_;
    Components::InputComp inputComp_;
    Components::DamageComp damageComp_;
    Components::HealComp healComp_;
    Components::ProgressComp progressComp_;

    bool undestroyable_;

    /// Effects may influence the cast spells speed
    float castSpeedFactor_ = 1.0f;
    /// For any skill
    float skillSpeedFactor_ = 1.0f;
    /// Effects may influence the attack speed
    float attackSpeedFactor_ = 1.0f;

    bool Serialize(IO::PropWriteStream& stream) override;
    void WriteSpawnData(Net::NetworkMessage& msg) override;
};

}
