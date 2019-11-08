#pragma once

#include "GameObject.h"
#include "Script.h"
#include "Skill.h"

namespace Game {

class Actor;

class AreaOfEffect final : public GameObject
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionEnded = 1 << 1,
        FunctionOnTrigger = 1 << 2,
        FunctionOnLeftArea = 1 << 3,
        FunctionOnCollide = 1 << 4,
    };
    std::weak_ptr<Actor> source_;
    kaguya::State luaState_;
    bool luaInitialized_{ false };
    std::shared_ptr<Script> script_;
    /// Effect or skill index
    uint32_t index_{ 0 };
    Ranges range_{ Ranges::Adjecent };
    uint32_t skillEffect_{ SkillEffectNone };
    uint32_t effectTarget_{ SkillTargetNone };
    uint32_t functions_{ FunctionNone };
    int64_t startTime_;
    // Lifetime
    uint32_t lifetime_;
    uint32_t itemIndex_{ 0 };
    bool HaveFunction(Function func) const
    {
        return luaInitialized_ && ((functions_ & func) == func);
    }
    void InitializeLua();
    void _LuaSetSource(Actor* source);
    Actor* _LuaGetSource();
private:
    // Events
    void OnCollide(GameObject* other);
    void OnTrigger(GameObject* other);
    void OnLeftArea(GameObject* other);
public:
    static void RegisterLua(kaguya::State& state);

    AreaOfEffect();
    ~AreaOfEffect() override;
    // non-copyable
    AreaOfEffect(const AreaOfEffect&) = delete;
    AreaOfEffect& operator=(const AreaOfEffect&) = delete;

    bool LoadScript(const std::string& fileName);
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::ObjectTypeAreaOfEffect;
    }
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    /// Collision shape type
    void SetShapeType(Math::ShapeType shape);
    Math::ShapeType GetShapeType() const;
    void SetRange(Ranges range);
    Ranges GetRange() const { return range_; }
    void SetSource(std::shared_ptr<Actor> source);
    std::shared_ptr<Actor> GetSource();
    uint32_t GetLifetime() const { return lifetime_; }
    void SetLifetime(uint32_t value) { lifetime_ = value; }
    int64_t GetStartTime() const { return startTime_; }
    uint32_t GetIndex() const { return index_; }
    void SetIndex(uint32_t value) { index_ = value; }
    bool HasEffect(SkillEffect effect) const { return (skillEffect_ & effect) == effect; }
    bool HasTarget(SkillTarget t) const { return (effectTarget_ & t) == t; }

    uint32_t GetGroupId() const;
    bool IsEnemy(const Actor* other) const;
    bool IsAlly(const Actor* other) const;
    bool IsInRange(const Actor* other) const;
    uint32_t GetItemIndex() const;
    void SetItemIndex(uint32_t value) { itemIndex_ = value; }

    bool Serialize(IO::PropWriteStream& stream) override;
    void WriteSpawnData(Net::NetworkMessage& msg) override;
};

template <>
inline bool Is<AreaOfEffect>(const GameObject& obj)
{
    return obj.GetType() == AB::GameProtocol::ObjectTypeAreaOfEffect;
}

}
