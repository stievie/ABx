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

#include "Actor.h"
#include <abscommon/Utils.h>
#include <sa/Bits.h>
#include <eastl.hpp>
#include <sa/time.h>

namespace Game {

class Item;

/// Projectile is an Actor and has an Item. Both have a script. This is similar to ItemDrop:
/// it is a GameObject and wraps an Item.
class Projectile final : public Actor
{
private:
    static const uint32_t DEFAULT_LIFETIME = 5000;
    ea::unique_ptr<Item> item_;
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
    Math::Vector3 startPos_;
    Math::Vector3 targetPos_;
    uint32_t targetMoveDir_{ AB::GameProtocol::MoveDirectionNone };
    float startDistance_{ std::numeric_limits<float>::max() };
    float minDistance_{ std::numeric_limits<float>::max() };
    ea::weak_ptr<Actor> source_;
    ea::weak_ptr<Actor> target_;
    bool started_{ false };
    uint32_t itemIndex_{ 0 };
    std::string itemUuid_;
    int64_t startTick_{ 0 };
    uint32_t lifeTime_{ DEFAULT_LIFETIME };
    uint32_t functions_{ FunctionNone };
    AB::GameProtocol::AttackError error_{ AB::GameProtocol::AttackError::None };
    void InitializeLua();
    bool LoadScript(const std::string& fileName);
    bool HaveFunction(Function func) const
    {
        return luaInitialized_ && sa::bits::is_set(functions_, func);
    }
    void SetError(AB::GameProtocol::AttackError error);
    Actor* _LuaGetSource();
    Actor* _LuaGetTarget();
    bool DoStart();
    bool IsExpired() const
    {
        if (startTick_ == 0)
            return false;
        return sa::time::is_expired(startTick_ + lifeTime_);
    }
private:
    void OnCollide(GameObject* other);
public:
    static void RegisterLua(kaguya::State& state);

    explicit Projectile(const std::string& itemUuid);
    Projectile() = delete;
    ~Projectile() override;

    bool Load();
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::GameObjectType::Projectile;
    }
    void SetSource(ea::shared_ptr<Actor> source);
    ea::shared_ptr<Actor> GetSource() const { return source_.lock(); }
    void SetTarget(ea::shared_ptr<Actor> target);
    ea::shared_ptr<Actor> GetTarget() const { return target_.lock(); }
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    uint32_t GetItemIndex() const override { return itemIndex_; }
    uint32_t GetLevel() const override;
    void SetLifeTime(uint32_t value) { lifeTime_ = value; }
    uint32_t GetLifeTime() const { return lifeTime_; }
};

template <>
inline bool Is<Projectile>(const GameObject& obj)
{
    return obj.GetType() == AB::GameProtocol::GameObjectType::Projectile;
}

}
