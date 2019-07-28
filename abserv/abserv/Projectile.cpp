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
        startPos_ = source->transformation_.position_ + HeadOffset;
        transformation_.position_ = startPos_;
        moveComp_->moved_ = true;
    }
}

void Projectile::SetTarget(std::shared_ptr<Actor> target)
{
    // Can not change target
    if (started_)
        return;
    target_ = target;
    targetMoveDir_ = target->moveComp_->moveDir_;
    targetPos_ = target->transformation_.position_ + BodyOffset;
    moveComp_->HeadTo(targetPos_);
    moveComp_->directionSet_ = true;

    distance_ = GetPosition().Distance(targetPos_);

    bool obstructed = false;
    std::vector<GameObject*> objects;
    const auto& origin = GetPosition();
    const auto direction = targetPos_ - origin;
    if (Raycast(objects, origin, direction, distance_))
    {
#ifdef DEBUG_COLLISION
        GameObject* obstructedBy = nullptr;
#endif
        // The target is also in this list
        for (const auto* o : objects)
        {
            if (o->id_ != id_ && o->id_ != target->id_)
            {
                obstructed = true;
#ifdef DEBUG_COLLISION
                obstructedBy = o;
#endif
                break;
            }
        }
#ifdef DEBUG_COLLISION
        if (obstructed)
            LOG_DEBUG << "Obstructed by " << obstructedBy->GetName() << std::endl;
#endif
    }

    else if (obstructed)
    {
        SetError(AB::GameProtocol::AttackErrorTargetObstructed);
    }
    currentDistance_ = distance_;
    started_ = OnStart();
    if (started_)
    {
        stateComp_.SetState(AB::GameProtocol::CreatureStateMoving, true);
    }
}

void Projectile::SetError(AB::GameProtocol::AttackError error)
{
    error_ = error;
    if (auto source = source_.lock())
        source->attackComp_.SetAttackError(error);
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
    if (error_ != AB::GameProtocol::AttackErrorNone)
    {
        Remove();
        return;
    }
    if (!started_)
        return;

    if (auto target = target_.lock())
    {
        if (targetMoveDir_ == target->moveComp_->moveDir_)
        {
            // If the target does not change direction we follow him.
            // Target can only doge when changing the direction after we lauched.
            targetPos_ = target->GetPosition();
            currentDistance_ = GetPosition().Distance(targetPos_);
        }
    }

    moveComp_->StoreOldPosition();
    moveComp_->HeadTo(targetPos_);
    moveComp_->directionSet_ = true;

    const float speed = moveComp_->GetSpeed(timeElapsed, BASE_MOVE_SPEED);
    moveComp_->Move(speed, Math::Vector3::UnitZ);

    // Adjust Y
    transformation_.position_.y_ = Math::Lerp(startPos_.y_,
        targetPos_.y_,
        GetPosition().Distance(targetPos_) / distance_);
    const Math::Vector3 velocity = moveComp_->CalculateVelocity(timeElapsed);
    Actor::Update(timeElapsed, message);

    if (item_)
        item_->Update(timeElapsed);

    auto source = source_.lock();
    if (auto target = target_.lock())
    {
        const float dist = GetPosition().Distance(targetPos_);
        if (dist < (velocity.Length() / static_cast<float>(timeElapsed)) * 10.0f)
        {
            // We may not really collide because of the low game update rate, so let's
            // approximate if we would collide
            if (error_ == AB::GameProtocol::AttackErrorNone)
                OnCollide(target.get());
            else
                Remove();
        }
        else if (dist < currentDistance_)
            currentDistance_ = dist;
        else if (dist > currentDistance_)
        {
            SetError(AB::GameProtocol::AttackErrorTargetDodge);
            Remove();
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

    if (other)
    {
        if (auto spt = target_.lock())
        {
            if (other->id_ == spt->id_)
            {
                if (HaveFunction(FunctionOnHitTarget))
                    ScriptManager::CallFunction(luaState_, "onHitTarget", other);
            }
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
