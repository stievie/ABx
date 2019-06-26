#pragma once

#include "Actor.h"
#include "Script.h"
#include "Ray.h"

namespace Game {

class Projectile : public Actor
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionOnCollide = 1 << 1,
        FunctionOnHitTarget = 1 << 2,
    };
    kaguya::State luaState_;
    bool luaInitialized_;
    bool startSet_;
    std::shared_ptr<Script> script_;
    Math::Vector3 start_;
    Math::Vector3 direction_;
    Math::Ray ray_;
    std::weak_ptr<Actor> source_;
    std::weak_ptr<Actor> target_;
    float speed_{ 0.0f };
    uint32_t functions_{ FunctionNone };
    void InitializeLua();
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
public:
    static void RegisterLua(kaguya::State& state);

    Projectile();
    // non-copyable
    Projectile(const Projectile&) = delete;
    Projectile& operator=(const Projectile&) = delete;

    bool LoadScript(const std::string& fileName);
    void SetSource(std::shared_ptr<Actor> source);
    void SetTarget(std::shared_ptr<Actor> target);
    void SetSpeed(float speed);
    float GetSpeed() const { return speed_; }
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    void OnCollide(GameObject* other) override;
};

}
