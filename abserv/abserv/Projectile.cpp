#include "stdafx.h"
#include "Projectile.h"
#include "ScriptManager.h"
#include "DataProvider.h"
#include "Game.h"

namespace Game {

void Projectile::RegisterLua(kaguya::State& state)
{
    state["Projectile"].setClass(kaguya::UserdataMetatable<Projectile, Actor>()
        .addFunction("GetSource", &Projectile::_LuaGetSource)
        .addFunction("GetTarget", &Projectile::_LuaGetTarget)
    );
}

Projectile::Projectile(const std::string& itemUuid) :
    Actor()
{
    SetCollisionShape(
        std::make_unique<Math::CollisionShapeImpl<Math::Sphere>>(Math::ShapeType::Sphere,
            Math::Vector3::Zero, PROJECTILE_SIZE)
    );
    // Projectile can not hide other objects
    occluder_ = false;
    undestroyable_ = true;
    selectable_ = false;
    itemUuid_ = itemUuid;

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

bool Projectile::Load()
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item gameItem;
    gameItem.uuid = itemUuid_;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Unable to read item with UUID " << itemUuid_ << std::endl;
        return false;
    }
    itemIndex_ = gameItem.index;
    return LoadScript(gameItem.actorScript);
}

void Projectile::SetSource(std::shared_ptr<Actor> source)
{
    if (!startSet_)
    {
        source_ = source;
        start_ = source->transformation_.position_ + HeadOffset;
        transformation_.position_ = start_;
    }
}

void Projectile::SetTarget(std::shared_ptr<Actor> target)
{
    // Can not change target
    if (started_)
        return;
    target_ = target;
    targetPos_ = target->transformation_.position_ + BodyOffset;
    HeadTo(targetPos_);
    moveComp_->HeadTo(targetPos_);
    ray_ = Math::Ray(start_, targetPos_ - start_);

    const auto targetBB = target->GetWorldBoundingBox();
    distance_ = ray_.HitDistance(targetBB);
    if (Math::IsInfinite(distance_))
    {
        if (auto source = source_.lock())
            source->attackComp_.SetAttackError(AB::GameProtocol::AttackErrorTargetObstructed);
        Remove();
        return;
    }
    currentDistance_ = distance_;
    started_ = OnStart();
}

Actor* Projectile::_LuaGetSource()
{
    auto o = GetSource();
    if (o)
        return o.get();
    return nullptr;
}

Actor* Projectile::_LuaGetTarget()
{
    auto o = GetTarget();
    if (o)
        return o.get();
    return nullptr;
}

void Projectile::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    if (started_)
    {
        moveComp_->Move(((float)(timeElapsed) / BASE_SPEED) * moveComp_->GetSpeedFactor(),
            Math::Vector3::UnitZ);
    }

    Actor::Update(timeElapsed, message);

    if (HaveFunction(FunctionUpdate))
        ScriptManager::CallFunction(luaState_, "onUpdate", timeElapsed);
    if (item_)
        item_->Update(timeElapsed);

    if (!started_)
        return;

    auto source = source_.lock();
    if (auto target = target_.lock())
    {
        const auto targetBB = target->GetWorldBoundingBox();
        assert(targetBB.IsDefined());
        assert(ray_.IsDefined());
        const float dist = ray_.HitDistance(targetBB);
        if (dist < currentDistance_)
            currentDistance_ = dist;
        else if (Math::IsInfinite(dist))
        {
            // No Hit
            if (source)
            {
                source->attackComp_.SetAttackError(AB::GameProtocol::AttackErrorTargetMissed);
                Remove();
                return;
            }
        }
        else if (!Math::Equals(currentDistance_, dist))
        {
            // Increasing distance
            OnCollide(target.get());
        }
    }
    else
    {
        // Target gone
        Remove();
    }
}

uint32_t Projectile::GetGroupId() const
{
    if (auto s = source_.lock())
        return s->GetGroupId();
    return 0u;
}

uint32_t Projectile::GetLevel() const
{
    if (auto s = source_.lock())
        return s->GetLevel();
    return 0u;
}

void Projectile::OnCollide(GameObject* other)
{
    Actor::OnCollide(other);
    if (HaveFunction(FunctionOnCollide))
        ScriptManager::CallFunction(luaState_, "onCollide", other);

    if (auto spt = target_.lock())
    {
        if (other && other->id_ == spt->id_)
        {
            if (HaveFunction(FunctionOnHitTarget))
                ScriptManager::CallFunction(luaState_, "onHitTarget", other);
        }
    }

    // Always remove when it collides with anything
    Remove();
}

bool Projectile::OnStart()
{
    auto t = target_.lock();
    assert(t);
    if (HaveFunction(FunctionOnStart))
    {
        bool ret = luaState_["onStart"](t.get());
        return ret;
    }
    return true;
}

}
