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

#include "stdafx.h"
#include "Projectile.h"
#include "ScriptManager.h"
#include "DataProvider.h"
#include "Game.h"
#include "Script.h"

namespace Game {

void Projectile::RegisterLua(kaguya::State& state)
{
    state["Projectile"].setClass(kaguya::UserdataMetatable<Projectile, Actor>()
        .addFunction("GetSource", &Projectile::_LuaGetSource)
        .addFunction("GetTarget", &Projectile::_LuaGetTarget)
        .addFunction("GetLifeTime", &Projectile::GetLifeTime)
        .addFunction("SetLifeTime", &Projectile::SetLifeTime)
        );
}

Projectile::Projectile(const std::string& itemUuid) :
    Actor()
{
    events_.Subscribe<void(GameObject*)>(EVENT_ON_COLLIDE, std::bind(&Projectile::OnCollide, this, std::placeholders::_1));
    SetCollisionShape(
        std::make_unique<Math::CollisionShape<Math::Sphere>>(Math::ShapeType::Sphere,
            Math::Vector3::Zero, PROJECTILE_SIZE)
    );
    // Projectile can not hide other objects
    occluder_ = false;
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
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
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

void Projectile::SetSource(std::shared_ptr<Actor> source)
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

    distance_ = currentDistance_ = GetPosition().Distance(targetPos_);

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
            if (o->id_ != target->id_)
            {
                obstructed = true;
#ifdef DEBUG_COLLISION
                obstructedBy = const_cast<GameObject*>(o);
#endif
                break;
            }
        }
#ifdef DEBUG_COLLISION
        if (obstructed)
            LOG_DEBUG << "Obstructed by " << *obstructedBy << std::endl;
#endif
    }

    if (obstructed)
    {
        SetError(AB::GameProtocol::AttackError::TargetObstructed);
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
    if (auto target = target_.lock())
    {
        if (targetMoveDir_ == target->moveComp_->moveDir_)
        {
            // If the target does not change direction we follow him.
            // Target can only dodge when changing the direction after we launched.
            targetPos_ = target->GetPosition() + BodyOffset;
            currentDistance_ = GetPosition().Distance(targetPos_);
        }
    }

    moveComp_->StoreOldPosition();
    if (currentDistance_ > 1.0f)
    {
        moveComp_->HeadTo(targetPos_);
        moveComp_->directionSet_ = true;
    }

    const float speed = moveComp_->GetSpeed(timeElapsed, BASE_MOVE_SPEED);
    moveComp_->Move(speed, Math::Vector3::UnitZ);
    // Adjust Y, also try to make a somewhat realistic trajectory
    const Math::Vector3 curPosOrigY = { transformation_.position_.x_, startPos_.y_, transformation_.position_.z_ };
    const float f = 1.0f - (curPosOrigY.Distance(targetPos_) / distance_);
    transformation_.position_.y_ = Math::Lerp(startPos_.y_, targetPos_.y_, f) +
        (sinf(f * Math::M_PIHALF) * ((distance_ / 10.0f) / speed));

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
            if (error_ == AB::GameProtocol::AttackError::None)
                CallEvent<void(GameObject*)>(EVENT_ON_COLLIDE, target.get());
        }
        else if (dist < currentDistance_)
            currentDistance_ = dist;
        else if (dist > currentDistance_)
        {
            SetError(AB::GameProtocol::AttackError::TargetDodge);
        }
        else if (dist > distance_ * 2.0f)
        {
            if (error_ == AB::GameProtocol::AttackError::None)
                SetError(AB::GameProtocol::AttackError::TargetMissed);
            Remove();
        }
    }
    else
    {
        // Target gone
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
                if (HaveFunction(FunctionOnHitTarget))
                    Lua::CallFunction(luaState_, "onHitTarget", other);
            }
        }
    }

    // Always remove when it collides with anything
    Remove();
}

bool Projectile::DoStart()
{
    auto t = target_.lock();
    assert(t);
    bool ret = true;
    if (HaveFunction(FunctionOnStart))
        ret = luaState_["onStart"](t.get());
    if (ret)
        startTick_ = Utils::Tick();
    return true;
}

}
