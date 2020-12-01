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

#include "Projectile.h"
#include "ScriptManager.h"
#include "DataProvider.h"
#include "Game.h"
#include "Script.h"

//#define DEBUG_COLLISION

namespace Game {

void Projectile::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Projectile"].setClass(kaguya::UserdataMetatable<Projectile, Actor>()
        .addFunction("GetSource", &Projectile::_LuaGetSource)
        .addFunction("GetTarget", &Projectile::_LuaGetTarget)
        .addFunction("GetLifeTime", &Projectile::GetLifeTime)
        .addFunction("SetLifeTime", &Projectile::SetLifeTime)
    );
    // clang-format on
}

Projectile::Projectile(const std::string& itemUuid) :
    Actor()
{
    events_.Subscribe<void(GameObject*)>(EVENT_ON_COLLIDE, std::bind(&Projectile::OnCollide, this, std::placeholders::_1));
    SetCollisionShape(
        ea::make_unique<Math::CollisionShape<Math::Sphere>>(Math::ShapeType::Sphere,
            Math::Vector3::Zero, PROJECTILE_SIZE)
    );
    // Projectile can not hide other objects
    SetOccluder(false);
    undestroyable_ = true;
    selectable_ = false;
    itemUuid_ = itemUuid;

    InitializeLua();
}

Projectile::~Projectile() = default;

void Projectile::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
    luaInitialized_ = true;
}

bool Projectile::LoadScript(const std::string& fileName)
{
    auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script)
        return false;
    if (!script->Execute(luaState_))
        return false;

    if (Lua::IsFunction(luaState_, "onCollide"))
        sa::bits::set(functions_, FunctionOnCollide);
    if (Lua::IsFunction(luaState_, "onHitTarget"))
        sa::bits::set(functions_, FunctionOnHitTarget);
    if (Lua::IsFunction(luaState_, "onStart"))
        sa::bits::set(functions_, FunctionOnStart);

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

void Projectile::SetSource(ea::shared_ptr<Actor> source)
{
    if (!startSet_)
    {
        source_ = source;
        SetGroupId(source->GetGroupId());
        startPos_ = source->transformation_.position_ + HeadOffset;
        transformation_.position_ = startPos_;
        moveComp_->moved_ = true;
    }
}

void Projectile::SetTarget(ea::shared_ptr<Actor> target)
{
    // Can not change target
    if (started_)
        return;
    target_ = target;
    targetMoveDir_ = target->moveComp_->moveDir_;
    targetPos_ = target->transformation_.position_ + BodyOffset;
    moveComp_->HeadTo(targetPos_);
    moveComp_->directionSet_ = true;

    startDistance_ = minDistance_ = GetPosition().DistanceXZ(targetPos_);
    float dist = GetPosition().Distance(targetPos_);

    ea::vector<GameObject*> objects;
    const auto& origin = GetPosition();
    const auto direction = targetPos_ - origin;
    if (Raycast(objects, origin, direction, dist))
    {
        // The target is also in this list
        for (const auto* o : objects)
        {
            if (o->id_ != target->id_)
            {
#ifdef DEBUG_COLLISION
                LOG_DEBUG << "Obstructed by " << *o << std::endl;
#endif
                // FIXME: TerrainPatch ray cast doesn't work great
//                SetError(AB::GameProtocol::AttackError::TargetObstructed);
                break;
            }
        }
    }

    started_ = DoStart();
    if (started_)
    {
        stateComp_.SetState(AB::GameProtocol::CreatureState::Moving, true);
    }
}

void Projectile::SetError(AB::GameProtocol::AttackError error)
{
    error_ = error;
    if (auto source = source_.lock())
        source->attackComp_->SetAttackError(error);
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
    if (!started_)
        return;

    if (IsExpired())
    {
        Remove();
        return;
    }
    auto target = target_.lock();
    if (!target)
    {
        Remove();
        return;

    }

    if (target->moveComp_->IsMoving())
    {
        if (targetMoveDir_ == target->moveComp_->moveDir_)
        {
            // If the target does not change direction we follow him.
            // Target can only dodge when changing the direction after we launched.
            const float newDist = GetPosition().DistanceXZ(targetPos_);
            if (newDist > 2.0f)
            {
                targetPos_ = target->GetPosition() + BodyOffset;
                minDistance_ = newDist;
            }
        }
        else
        {
            SetError(AB::GameProtocol::AttackError::TargetDodge);
        }
    }

    moveComp_->StoreOldPosition();
    if (minDistance_ > 2.0f)
    {
        moveComp_->HeadTo(targetPos_);
        moveComp_->directionSet_ = true;
    }

    const float speed = moveComp_->GetSpeed(timeElapsed, BASE_MOVE_SPEED);
    moveComp_->Move(speed, Math::Vector3::UnitZ);
    // Adjust Y, also try to make a somewhat realistic trajectory
    const float f = 1.0f - (minDistance_ / startDistance_);
    transformation_.position_.y_ = Math::Lerp(startPos_.y_, targetPos_.y_, f) +
        (sinf(f * Math::M_PIF) * ((startDistance_ / 10.0f) / speed));

    const Math::Vector3 velocity = moveComp_->CalculateVelocity(timeElapsed);
    Actor::Update(timeElapsed, message);

    if (item_)
        item_->Update(timeElapsed);

    const float dist = GetPosition().DistanceXZ(targetPos_);
    if (error_ == AB::GameProtocol::AttackError::TargetDodge)
    {
        Remove();
    }
    else if (dist < (velocity.Length() / static_cast<float>(timeElapsed)) * 5.0f)
    {
        // We may not really collide because of the low game update rate, so let's
        // approximate if we would collide
        if (error_ == AB::GameProtocol::AttackError::None)
            CallEvent<void(GameObject*)>(EVENT_ON_COLLIDE, target.get());
        else
            Remove();
    }
    else if (dist < minDistance_)
        minDistance_ = dist;
    else if (dist > startDistance_)
    {
        if (error_ == AB::GameProtocol::AttackError::None)
            SetError(AB::GameProtocol::AttackError::TargetMissed);
        Remove();
    }
}

uint32_t Projectile::GetLevel() const
{
    if (auto s = source_.lock())
        return s->GetLevel();
    return 0u;
}

void Projectile::OnCollide(GameObject* other)
{
    if (HaveFunction(FunctionOnCollide))
        Lua::CallFunction(luaState_, "onCollide", other);

    if (other)
    {
        if (auto spt = target_.lock())
        {
            if (other->id_ == spt->id_)
            {
                if (error_ == AB::GameProtocol::AttackError::None)
                {
                    if (HaveFunction(FunctionOnHitTarget))
                        Lua::CallFunction(luaState_, "onHitTarget", other);
                }
            }
        }
    }

    // Always remove when it collides with anything
    Remove();
}

bool Projectile::DoStart()
{
    auto t = target_.lock();
    ASSERT(t);
    bool ret = true;
    if (HaveFunction(FunctionOnStart))
        ret = luaState_["onStart"](t.get());
    if (ret)
        startTick_ = sa::time::tick();
    return true;
}

}
