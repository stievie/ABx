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
#include "InventoryComp.h"
#include "SkillsComp.h"
#include "InputComp.h"
#include "DamageComp.h"
#include "HealComp.h"
#include "ProgressComp.h"
#include "UuidUtils.h"

namespace Game {

/// 8 different groups in one game possible
enum GroupMask : uint8_t
{
    GroupMaskNone = 0,
    GroupMask1 = 1,
    GroupMask2 = 1 << 1,
    GroupMask3 = 1 << 2,
    GroupMask4 = 1 << 3,
    GroupMask5 = 1 << 4,
    GroupMask6 = 1 << 5,
    GroupMask7 = 1 << 6,
    GroupMask8 = 1 << 7
};

/// Player, NPC, Monster some such
class Actor : public GameObject
{
    friend class Skill;
    friend class Components::MoveComp;
    friend class Components::AutoRunComp;
    friend class Components::CollisionComp;
    friend class Components::ResourceComp;
    friend class Components::AttackComp;
    friend class Components::EffectsComp;
    friend class Components::InventoryComp;
    friend class Components::SkillsComp;
    friend class Components::InputComp;
    friend class Components::DamageComp;
    friend class Components::HealComp;
    friend class Components::ProgressComp;
private:
    void _LuaGotoPosition(const Math::STLVector3& pos);
    int _LuaGetState();
    void _LuaSetState(int state);
    void _LuaSetHomePos(const Math::STLVector3& pos);
    void _LuaHeadTo(const Math::STLVector3& pos);
    Math::STLVector3 _LuaGetHomePos();
    void _LuaFollowObject(GameObject* object);
    void _LuaAddEffect(Actor* source, uint32_t index, uint32_t time);
    void _LuaRemoveEffect(uint32_t index);
    uint8_t _LuaGetGroupMask();
    void _LuaSetGroupMask(uint8_t value);
    Effect* _LuaGetLastEffect(AB::Entities::EffectCategory category);
    GameObject* _LuaGetSelectedObject();
    void _LuaSetSelectedObject(GameObject* object);
protected:
    Math::Vector3 homePos_;
    std::weak_ptr<Actor> killedBy_;
    virtual void HandleCommand(AB::GameProtocol::CommandTypes,
        const std::string&, Net::NetworkMessage&)
    {
        // Only the player needs to override this
    }
protected:
    // Mostly called by Components, which are friends
    virtual void OnArrived() {}
    virtual void OnInterruptedAttack() { }
    virtual void OnInterruptedSkill(Skill*) { }
    virtual void OnKnockedDown(uint32_t) { }
    virtual void OnHealed(int) { }
    virtual void OnDied();
    virtual void OnResurrected(int /* health */, int /* energy */) { }
    virtual void OnPingObject(uint32_t /* targetId */, AB::GameProtocol::ObjectCallType /* type */, int /* skillIndex */) { }
    virtual void OnInventoryFull() { }
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
    virtual void OnEndUseSkill(Skill* skill);
    virtual void OnStartUseSkill(Skill* skill);
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
    void VisitEnemiesInRange(Ranges range, Func&& func)
    {
        VisitInRange(range, [&](const GameObject& o)
        {
            const AB::GameProtocol::GameObjectType t = o.GetType();
            if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
            {
                const auto* actor = dynamic_cast<const Actor*>(&o);
                if (actor && actor->IsEnemy(this))
                    func(actor);
            }
            return Iteration::Continue;
        });
    }
    std::vector<Actor*> GetEnemiesInRange(Ranges range);
    size_t GetEnemyCountInRange(Ranges range);
    /// Ally is always without self
    template<typename Func>
    void VisitAlliesInRange(Ranges range, Func&& func)
    {
        VisitInRange(range, [&](const GameObject& o)
        {
            const AB::GameProtocol::GameObjectType t = o.GetType();
            if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
            {
                const auto* actor = dynamic_cast<const Actor*>(&o);
                if (actor && !actor->IsEnemy(this))
                    func(actor);
            }
            return Iteration::Continue;
        });
    }
    std::vector<Actor*> GetAlliesInRange(Ranges range);
    size_t GetAllyCountInRange(Ranges range);
    Actor* GetClosestEnemy(bool undestroyable, bool unselectable);
    Actor* GetClosestAlly(bool undestroyable, bool unselectable);
    Item* GetWeapon() const;

    void HeadTo(const Math::Vector3& pos);
    void FaceObject(GameObject* object);
    /// Move speed: 1 = normal speed
    float GetSpeed() const { return moveComp_->GetSpeedFactor(); }
    void SetSpeed(float value) { moveComp_->SetSpeedFactor(value); }
    void AddSpeed(float value) { moveComp_->AddSpeed(value); }
    bool IsMoving() const { return moveComp_->IsMoving(); }
    /// Attack hit
    bool IsHitting() const { return attackComp_.IsHitting(); }
    bool IsUndestroyable() const { return undestroyable_; }
    void SetUndestroyable(bool value) { undestroyable_ = value; }
    bool IsInWeaponRange(Actor* actor) const;
    /// Get effect of armor. Armor is influenced by the equipment and effects
    /// Damage multiplier.
    float GetArmorEffect(DamageType damageType, DamagePos pos, float penetration);
    uint32_t GetAttackSpeed();
    DamageType GetAttackDamageType();
    int32_t GetAttackDamage(bool critical);
    float GetArmorPenetration();
    /// Get chance for a critical hit. Value between 0..1
    float GetCriticalChance(const Actor* other);
    DamagePos GetDamagePos() const { return damageComp_.GetDamagePos(); }
    int GetResource(Components::ResourceType type) const { return resourceComp_.GetValue(type); }
    void SetResource(Components::ResourceType type, Components::SetValueType t, int value);
    /// Adds damage to this actor, skill or effect index my be 0.
    void ApplyDamage(Actor* source, uint32_t index, DamageType type, int value, float penetration);
    /// Steal life from this actor. The source must add the returned value to its life.
    int DrainLife(Actor* source, uint32_t index, int value);
    /// Steal energy from this actor. The source must add the returned value to its energy.
    int DrainEnergy(int value);
    void SetHealthRegen(int value);

    virtual bool OnInterruptingAttack();
    virtual bool OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill);
    bool InterruptAttack();
    bool InterruptSkill(AB::Entities::SkillType type);
    /// Interrupt everything
    bool Interrupt();

    virtual bool CanAttack() const { return false; }
    virtual bool CanUseSkill() const { return false; }
    virtual uint32_t GetLevel() const { return 0; }
    virtual void SetLevel(uint32_t) { }
    virtual void AddXp(int) { }
    virtual uint32_t GetXp() const { return 0; }
    virtual void AddSkillPoint() { }
    virtual uint32_t GetSkillPoints() const { return 0; }
    virtual void AdvanceLevel();
    virtual AB::Entities::CharacterSex GetSex() const
    {
        return AB::Entities::CharacterSexUnknown;
    }
    virtual uint32_t GetItemIndex() const
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
    bool SetInventory(const std::string& ciUuid);
    bool SetChest(const std::string& ciUuid);
    virtual const std::string& GetPlayerUuid() const { return Utils::Uuid::EMPTY_UUID; }
    virtual const std::string& GetAccountUuid() const { return Utils::Uuid::EMPTY_UUID; }

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    virtual bool Die();
    virtual bool Resurrect(int precentHealth, int percentEnergy);
    bool IsDead() const { return stateComp_.IsDead(); }
    bool IsKnockedDown() const { return stateComp_.IsKnockedDown(); }
    /// Returns true when the actor can't do anything
    bool IsImmobilized() const { return stateComp_.IsDead() || stateComp_.IsKnockedDown(); }
    /// Knock the Actor down caused by source. If time = 0 DEFAULT_KNOCKDOWN_TIME is used.
    bool KnockDown(Actor* source, uint32_t time);
    int Healing(Actor* source, uint32_t index, int value);
    bool IsEnemy(Actor* other) const;
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
    void SetSelectedObjectById(uint32_t id);
    void GotoPosition(const Math::Vector3& pos);
    void FollowObject(std::shared_ptr<GameObject> object);
    void FollowObject(uint32_t objectId);
    void UseSkill(uint32_t index);
    void Attack(Actor* target);
    bool AttackById(uint32_t targetId);
    bool IsAttackingActor(Actor* target);
    /// Cancel attack, use skill, follow
    void CancelAction();

    virtual bool AddToInventory(std::unique_ptr<Item>& item);
    void DropRandomItem();

    std::unique_ptr<SkillBar> skills_;

    Components::AutoRunComp autorunComp_;
    Components::ResourceComp resourceComp_;
    Components::AttackComp attackComp_;
    Components::SkillsComp skillsComp_;
    Components::InputComp inputComp_;
    Components::DamageComp damageComp_;
    Components::HealComp healComp_;
    std::unique_ptr<Components::ProgressComp> progressComp_;
    std::unique_ptr<Components::EffectsComp> effectsComp_;
    std::unique_ptr<Components::InventoryComp> inventoryComp_;
    std::unique_ptr<Components::MoveComp> moveComp_;
    std::unique_ptr<Components::CollisionComp> collisionComp_;

    bool undestroyable_{ false };
    uint8_t groupMask_{ GroupMask::GroupMaskNone };

    bool Serialize(IO::PropWriteStream& stream) override;
    void WriteSpawnData(Net::NetworkMessage& msg) override;
};

}
