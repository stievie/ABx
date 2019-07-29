#pragma once

#include "Actor.h"
#include "Script.h"
#include "Ray.h"

namespace Game {

class Item;

/// Projectile is an Actor and has an Item. Both have a script. This is similar to ItemDrop:
/// it is a GameObject and wraps an Item.
class Projectile final : public Actor
{
private:
    std::unique_ptr<Item> item_;
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionOnCollide = 1 << 1,
        FunctionOnHitTarget = 1 << 2,
        FunctionOnStart = 1 << 3,
    };
    kaguya::State luaState_;
    bool luaInitialized_{ false };
    bool startSet_{ false };
    std::shared_ptr<Script> script_;
    Math::Vector3 startPos_;
    Math::Vector3 targetPos_;
    uint32_t targetMoveDir_{ AB::GameProtocol::MoveDirectionNone };
    float distance_{ std::numeric_limits<float>::max() };
    float currentDistance_{ std::numeric_limits<float>::max() };
    std::weak_ptr<Actor> source_;
    std::weak_ptr<Actor> target_;
    bool started_{ false };
    uint32_t itemIndex_{ 0 };
    std::string itemUuid_;
    uint32_t functions_{ FunctionNone };
    AB::GameProtocol::AttackError error_{ AB::GameProtocol::AttackErrorNone };
    void InitializeLua();
    bool LoadScript(const std::string& fileName);
    bool HaveFunction(Function func) const
    {
        return luaInitialized_ && ((functions_ & func) == func);
    }
    void SetError(AB::GameProtocol::AttackError error);
    Actor* _LuaGetSource();
    Actor* _LuaGetTarget();
protected:
    void OnCollide(GameObject* other) override;
    bool OnStart();
public:
    static void RegisterLua(kaguya::State& state);

    explicit Projectile(const std::string& itemUuid);
    Projectile() = delete;
    // non-copyable
    Projectile(const Projectile&) = delete;
    Projectile& operator=(const Projectile&) = delete;

    bool Load();
    AB::GameProtocol::GameObjectType GetType() const final override
    {
        return AB::GameProtocol::ObjectTypeProjectile;
    }
    void SetSource(std::shared_ptr<Actor> source);
    std::shared_ptr<Actor> GetSource() const { return source_.lock(); }
    void SetTarget(std::shared_ptr<Actor> target);
    std::shared_ptr<Actor> GetTarget() const { return target_.lock(); }
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    uint32_t GetGroupId() const override;
    uint32_t GetItemIndex() const override { return itemIndex_; }
    uint32_t GetLevel() const override;
};

}
