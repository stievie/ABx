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
private:
    void _LuaGotoPosition(float x, float y, float z);
    int _LuaGetState();
    void _LuaSetState(int state);
    void _LuaSetHomePos(float x, float y, float z);
    void _LuaHeadTo(float x, float y, float z);
    std::vector<float> _LuaGetHomePos();
    void _LuaFollowObject(std::shared_ptr<GameObject> object);
    std::vector<std::shared_ptr<Actor>> _LuaGetActorsInRange(Ranges range);
    void _LuaAddEffect(std::shared_ptr<Actor> source, uint32_t index);
    void _LuaRemoveEffect(uint32_t index);
    std::shared_ptr<Effect> _LuaGetLastEffect(AB::Entities::EffectCategory category);
    void UpdateRanges();
protected:
    std::vector<Math::Vector3> wayPoints_;
    std::map<Ranges, std::vector<GameObject*>> ranges_;

    /// Time in ms the same Actor can retrigger
    uint32_t retriggerTimeout_;
    std::map<uint32_t, int64_t> triggered_;
    /// If true fires onTrigger
    bool trigger_;
    Math::Vector3 homePos_;
    virtual void HandleCommand(AB::GameProtocol::CommandTypes type,
        const std::string& command, Net::NetworkMessage& message) {
        AB_UNUSED(type);
        AB_UNUSED(command);
        AB_UNUSED(message);
    }
    virtual void OnArrived() {}
public:
    static void RegisterLua(kaguya::State& state);

    Actor();

    void SetGame(std::shared_ptr<Game> game) override
    {
        GameObject::SetGame(game);
    }

    void SetHomePos(const Math::Vector3& pos)
    {
        homePos_ = pos;
    }
    const Math::Vector3& GetHomePos() const { return homePos_; }
    bool GotoHomePos();
    uint32_t GetRetriggerTimout() const
    {
        return retriggerTimeout_;
    }
    void SetRetriggerTimout(uint32_t value)
    {
        retriggerTimeout_ = value;
    }
    bool IsTrigger() const
    {
        return trigger_;
    }
    void SetTrigger(bool value)
    {
        trigger_ = value;
    }
    /**
    * @brief Allows to execute a functor/lambda on the visible objects
    * @note This is thread safe
    */
    template<typename Func>
    void VisitInRange(Ranges range, Func&& func)
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (const auto o : ranges_[range])
            func(o);
    }
    bool IsInRange(Ranges range, Actor* actor)
    {
        if (!actor)
            return false;
        if (range == Ranges::Map)
            return true;
        std::lock_guard<std::mutex> lock(lock_);
        for (const auto& o : ranges_[range])
            if (o->id_ == actor->id_)
                return true;
        return false;
    }
    virtual void OnEndUseSkill(Skill* skill);
    virtual void OnStartUseSkill(Skill* skill);

    void HeadTo(const Math::Vector3& pos);
    void FaceObject(GameObject* object);
    /// Move speed: 1 = normal speed
    float GetSpeed() const { return moveComp_.GetSpeedFactor(); }
    void SetSpeed(float value) { moveComp_.SetSpeedFactor(value); }
    void AddSpeed(float value) { moveComp_.AddSpeed(value); }
    bool IsUndestroyable() const { return undestroyable_; }
    void SetUndestroyable(bool value) { undestroyable_ = value; }

    virtual uint32_t GetLevel() const { return 0; }

    virtual AB::Entities::CharacterSex GetSex() const
    {
        return AB::Entities::CharacterSexUnknown;
    }
    virtual uint32_t GetModelIndex() const
    {
        return 0;
    }
    virtual uint32_t GetGroupId() const { return 0; }
    uint32_t GetProfIndex() const
    {
        return skills_.prof1_.index;
    }
    uint32_t GetProf2Index() const
    {
        return skills_.prof2_.index;
    }
    /// 0 Based
    Skill* GetSkill(uint32_t index)
    {
        if (index < PLAYER_MAX_SKILLS)
            return skills_[index];
        return nullptr;
    }
    SkillBar* GetSkillBar()
    {
        return &skills_;
    }
    Skill* GetCurrentSkill() const;
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    virtual bool Die();
    virtual bool Resurrect(int16_t precentHealth, int16_t percentEnergy);
    bool IsDead() const { return stateComp_.IsDead(); }
    bool IsEnemy(Actor* other);
    inline void AddInput(InputType type, const Utils::VariantMap& data)
    {
        inputComp_.Add(type, data);
    }
    inline void AddInput(InputType type)
    {
        inputComp_.Add(type);
    }
    uint32_t GetAttributeValue(uint32_t index) const
    {
        const AB::AttributeValue* val = skills_.GetAttribute(index);
        if (val == nullptr)
            return 0;
        return val->value;
    }

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

    SkillBar skills_;

    Components::MoveComp moveComp_;
    Components::AutoRunComp autorunComp_;
    Components::CollisionComp collisionComp_;
    Components::ResourceComp resourceComp_;
    Components::AttackComp attackComp_;
    Components::EffectsComp effectsComp_;
    Components::EquipComp equipComp_;
    Components::SkillsComp skillsComp_;
    Components::InputComp inputComp_;

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
