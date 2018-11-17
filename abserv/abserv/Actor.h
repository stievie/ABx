#pragma once

#include "GameObject.h"
#include "Effect.h"
#include "SkillBar.h"
#include "InputQueue.h"
#include <AB/ProtocolCodes.h>
#include "MoveComp.h"
#include "AutoRunComp.h"
#include "CollisionComp.h"

namespace Game {

/// Player, NPC, Monster some such
class Actor : public GameObject
{
    friend class Components::MoveComp;
    friend class Components::AutoRunComp;
    friend class Components::CollisionComp;
public:
    static constexpr float MAX_INTERACTION_DIST = 1.0f;
    static constexpr float SWITCH_WAYPOINT_DIST = 2.0f;
private:
    void DeleteEffect(uint32_t index);
    void _LuaGotoPosition(float x, float y, float z);
    int _LuaGetState();
    void _LuaSetState(int state);
    void _LuaSetHomePos(float x, float y, float z);
    std::vector<float> _LuaGetHomePos();
protected:
    std::vector<Math::Vector3> wayPoints_;
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

    /// Move speed: 1 = normal speed
    float GetSpeed() const { return moveComp_.GetSpeedFactor(); }
    void SetSpeed(float value) { moveComp_.SetSpeedFactor(value); }
    uint32_t GetEnergy() const { return energy_; }
    void SetEnergy(uint32_t value) { energy_ = value; }
    uint32_t GetHealth() const { return health_; }
    void SetHealth(uint32_t value) { health_ = value; }
    uint32_t GetAdrenaline() const { return adrenaline_; }
    void SetAdrenaline(uint32_t value) { adrenaline_ = value; }
    uint32_t GetOvercast() const { return overcast_; }
    void SetOvercast(uint32_t value) { overcast_ = value; }
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
    uint32_t GetProfIndex() const
    {
        return skills_.prof1_.index;
    }
    uint32_t GetProf2Index() const
    {
        return skills_.prof2_.index;
    }
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
    void AddEffect(std::shared_ptr<Actor> source, uint32_t index, uint32_t baseDuration);
    /// Remove effect before it ended
    void RemoveEffect(uint32_t index);

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    bool Die();
    bool IsDead() const { return health_ == 0; }

    InputQueue inputs_;
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
    void UseSkill(uint32_t index);

    Components::MoveComp moveComp_;
    Components::AutoRunComp autorunComp_;
    Components::CollisionComp collisionComp_;

    EffectList effects_;
    SkillBar skills_;
    uint32_t energy_;
    uint32_t health_;
    uint32_t adrenaline_;
    uint32_t overcast_;
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
