#include "stdafx.h"
#include "Projectile.h"
#include "ScriptManager.h"
#include "DataProvider.h"
#include "Game.h"
#include "Mechanic.h"

namespace Game {

void Projectile::RegisterLua(kaguya::State& state)
{
    state["Projectile"].setClass(kaguya::UserdataMetatable<Projectile, Actor>()
        .addFunction("SetSpeed", &Projectile::SetSpeed)
        .addFunction("GetSpeed", &Projectile::GetSpeed)
    );
}

Projectile::Projectile() :
    Actor(),
    luaInitialized_(false),
    startSet_(false)
{
    // AOE has always sphere shape with the range as radius
    SetCollisionShape(
        std::make_unique<Math::CollisionShapeImpl<Math::Sphere>>(Math::ShapeTypeSphere,
            Math::Vector3::Zero, PROJECTILE_SIZE)
    );
    // Projectile can not hide other objects
    occluder_ = false;

    InitializeLua();
}

void Projectile::InitializeLua()
{
    ScriptManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
    luaInitialized_ = true;
}

bool Projectile::LoadScript(const std::string& fileName)
{
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;
    if (ScriptManager::IsFunction(luaState_, "onUpdate"))
        functions_ |= FunctionUpdate;
    if (ScriptManager::IsFunction(luaState_, "onCollide"))
        functions_ |= FunctionOnCollide;
    if (ScriptManager::IsFunction(luaState_, "onHitTarget"))
        functions_ |= FunctionOnHitTarget;
    if (ScriptManager::IsFunction(luaState_, "onStart"))
        functions_ |= FunctionOnStart;

    bool ret = luaState_["onInit"]();
    return ret;
}

void Projectile::SetSource(std::shared_ptr<Actor> source)
{
    if (!startSet_)
    {
        source_ = source;
        start_ = source->transformation_.position_;
        transformation_.position_ = start_;
    }
}

void Projectile::SetTarget(std::shared_ptr<Actor> target)
{
    assert(startSet_);
    // Can not change target
    assert(!target);
    target_ = target;
    direction_ = target->transformation_.position_;
    ray_ = Math::Ray(start_, direction_);
    // TODO: Raycast query
}

void Projectile::SetSpeed(float speed)
{
    if (speed_ != speed)
    {
        speed_ = speed;
        moveComp_->SetSpeedFactor(speed);
    }
}

void Projectile::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    if (!started_)
    {
        started_ = OnStart();
    }
    if (started_)
    {
        moveComp_->HeadTo(direction_);
        moveComp_->Move(((float)(timeElapsed) / BASE_SPEED) * moveComp_->GetSpeedFactor(),
            Math::Vector3::UnitZ);        
    }

    GameObject::Update(timeElapsed, message);

    if (luaInitialized_ && HaveFunction(FunctionUpdate))
        ScriptManager::CallFunction(luaState_, "onUpdate", timeElapsed);
}

void Projectile::OnCollide(GameObject* other)
{
    Actor::OnCollide(other);
    if (luaInitialized_ && HaveFunction(FunctionOnCollide))
        ScriptManager::CallFunction(luaState_, "onCollide", other);

    if (auto spt = target_.lock())
    {
        if (other && other->id_ == spt->id_)
        {
            if (luaInitialized_ && HaveFunction(FunctionOnHitTarget))
                ScriptManager::CallFunction(luaState_, "onHitTarget", other);
        }
    }

    // Always remove when it collides with anything
    GetGame()->RemoveObject(this);
}

bool Projectile::OnStart()
{
    auto t = target_.lock();
    assert(t);
    if (luaInitialized_ && HaveFunction(FunctionOnCollide))
    {
        bool ret = luaState_["onStart"](t.get());
        return ret;        
    }
    return true;
}

}
