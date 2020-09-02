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

#include "AreaOfEffect.h"
#include "Actor.h"
#include "DataProvider.h"
#include "Game.h"
#include "Script.h"
#include "ScriptManager.h"
#include <sa/time.h>

namespace Game {

void AreaOfEffect::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["AreaOfEffect"].setClass(kaguya::UserdataMetatable<AreaOfEffect, GameObject>()
        .addFunction("SetRange", &AreaOfEffect::SetRange)
        .addFunction("GetRange", &AreaOfEffect::GetRange)
        .addFunction("SetShapeType", &AreaOfEffect::SetShapeType)
        .addFunction("GetShapeType", &AreaOfEffect::GetShapeType)
        .addFunction("GetSource", &AreaOfEffect::_LuaGetSource)
        .addFunction("SetSource", &AreaOfEffect::_LuaSetSource)
        .addFunction("GetStartTime", &AreaOfEffect::GetStartTime)
        .addFunction("GetLifetime", &AreaOfEffect::GetLifetime)
        .addFunction("SetLifetime", &AreaOfEffect::SetLifetime)
        .addFunction("GetItemIndex", &AreaOfEffect::GetItemIndex)
        .addFunction("SetItemIndex", &AreaOfEffect::SetItemIndex)

        .addFunction("Index", &AreaOfEffect::GetIndex)
    );
    // clang-format on
}

AreaOfEffect::AreaOfEffect() :
    GameObject(),
    startTime_(sa::time::tick())
{
    events_.Subscribe<void(GameObject*)>(EVENT_ON_COLLIDE, std::bind(&AreaOfEffect::OnCollide, this, std::placeholders::_1));
    events_.Subscribe<void(GameObject*)>(EVENT_ON_TRIGGER, std::bind(&AreaOfEffect::OnTrigger, this, std::placeholders::_1));
    events_.Subscribe<void(GameObject*)>(EVENT_ON_LEFTAREA, std::bind(&AreaOfEffect::OnLeftArea, this, std::placeholders::_1));
    // By default AOE has a sphere shape with the range as radius
    SetCollisionShape(
        ea::make_unique<Math::CollisionShape<Math::Sphere>>(Math::ShapeType::Sphere,
            Math::Vector3::Zero, RangeDistances[static_cast<int>(range_)] * 0.5f)
    );
    // AOE can not hide other objects
    occluder_ = false;
    // AOE usually not colliding
    collisionMask_ = 0;
    selectable_ = false;

    InitializeLua();
}

AreaOfEffect::~AreaOfEffect() = default;

void AreaOfEffect::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
    luaInitialized_ = true;
}

void AreaOfEffect::_LuaSetSource(Actor* source)
{
    if (source)
        source_ = source->GetPtr<Actor>();
    else
        source_.reset();
}

Actor* AreaOfEffect::_LuaGetSource()
{
    return GetSource().get();
}

bool AreaOfEffect::LoadScript(const std::string& fileName)
{
    auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script)
        return false;
    if (!script->Execute(luaState_))
        return false;

    if (Lua::IsNumber(luaState_, "itemIndex"))
        itemIndex_ = luaState_["itemIndex"];
    else
        LOG_WARNING << "AOE " << fileName << " does not have an itemIndex" << std::endl;
    if (Lua::IsNumber(luaState_, "creatureState"))
        stateComp_.SetState(luaState_["creatureState"], true);
    else
        stateComp_.SetState(AB::GameProtocol::CreatureState::Idle, true);
    if (Lua::IsNumber(luaState_, "effect"))
        skillEffect_ = luaState_["effect"];
    if (Lua::IsNumber(luaState_, "effectTarget"))
        effectTarget_ = luaState_["effectTarget"];

    if (Lua::IsFunction(luaState_, "onUpdate"))
        sa::bits::set(functions_, FunctionUpdate);
    if (Lua::IsFunction(luaState_, "onEnded"))
        sa::bits::set(functions_, FunctionEnded);
    if (Lua::IsFunction(luaState_, "onCollide"))
        sa::bits::set(functions_, FunctionOnCollide);
    if (Lua::IsFunction(luaState_, "onTrigger"))
        sa::bits::set(functions_, FunctionOnTrigger);
    if (Lua::IsFunction(luaState_, "onLeftArea"))
        sa::bits::set(functions_, FunctionOnLeftArea);

    bool ret = luaState_["onInit"]();
    return ret;
}

void AreaOfEffect::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);

    stateComp_.Update(timeElapsed);

    stateComp_.Write(message);

    if (HaveFunction(FunctionUpdate))
        Lua::CallFunction(luaState_, "onUpdate", timeElapsed);
    if (sa::time::time_elapsed(startTime_) > lifetime_)
    {
        if (HaveFunction(FunctionEnded))
            Lua::CallFunction(luaState_, "onEnded");
        Remove();
    }
}

void AreaOfEffect::OnCollide(GameObject* other)
{
    // Called from collisionComp_ of the moving object
    // AOE can also be a trap for example
    if (HaveFunction(FunctionOnCollide))
        Lua::CallFunction(luaState_, "onCollide", other);
}

void AreaOfEffect::OnTrigger(GameObject* other)
{
    // AOE can also be a trap for example
    if (HaveFunction(FunctionOnTrigger))
        Lua::CallFunction(luaState_, "onTrigger", other);
}

void AreaOfEffect::OnLeftArea(GameObject* other)
{
    // AOE can also be a trap for example
    if (HaveFunction(FunctionOnLeftArea))
        Lua::CallFunction(luaState_, "onLeftArea", other);
}

Math::ShapeType AreaOfEffect::GetShapeType() const
{
    auto* shape = GetCollisionShape();
    if (!shape)
        return Math::ShapeType::None;
    return shape->shapeType_;
}

void AreaOfEffect::SetShapeType(Math::ShapeType shape)
{
    switch (shape)
    {
    case Math::ShapeType::BoundingBox:
    {
        const float rangeSize = RangeDistances[static_cast<int>(range_)] * 0.5f;
        const Math::Vector3 halfSize = { rangeSize, rangeSize, rangeSize };
        SetCollisionShape(
            ea::make_unique<Math::CollisionShape<Math::BoundingBox>>(Math::ShapeType::BoundingBox,
                -halfSize, halfSize)
        );
        break;
    }
    case Math::ShapeType::Sphere:
        SetCollisionShape(
            ea::make_unique<Math::CollisionShape<Math::Sphere>>(Math::ShapeType::Sphere,
                Math::Vector3::Zero, RangeDistances[static_cast<int>(range_)] * 0.5f)
        );
        break;
    case Math::ShapeType::None:
        SetCollisionShape(ea::unique_ptr<Math::AbstractCollisionShape>());
        break;
    default:
        LOG_ERROR << "Invalid shape type for AOE " << static_cast<int>(shape) << std::endl;
        break;
    }
}

void AreaOfEffect::SetRange(Ranges range)
{
    // Size can also be set with GameObject::(_Lua)SetBoundingSize()
    if (range_ == range)
        return;

    range_ = range;

    // Update collision shape size
    auto* cs = GetCollisionShape();
    if (!cs)
        return;

    switch (cs->shapeType_)
    {
    case Math::ShapeType::BoundingBox:
    {
        const float rangeSize = RangeDistances[static_cast<int>(range_)] * 0.5f;
        const Math::Vector3 halfSize = { rangeSize, rangeSize, rangeSize };
        using BoxShape = Math::CollisionShape<Math::BoundingBox>;
        BoxShape* shape = static_cast<BoxShape*>(cs);
        shape->Object().min_ = -halfSize;
        shape->Object().max_ = halfSize;
        break;
    }
    case Math::ShapeType::Sphere:
    {
        using SphereShape = Math::CollisionShape<Math::Sphere>;
        SphereShape* shape = static_cast<SphereShape*>(cs);
        shape->Object().radius_ = RangeDistances[static_cast<int>(range)] * 0.5f;
        break;
    }
    default:
        LOG_WARNING << "Can not set range for shape type " << static_cast<int>(cs->shapeType_) << std::endl;
        break;
    }
}

void AreaOfEffect::SetSource(ea::shared_ptr<Actor> source)
{
    source_ = source;
}

ea::shared_ptr<Actor> AreaOfEffect::GetSource()
{
    return source_.lock();
}

uint32_t AreaOfEffect::GetGroupId() const
{
    if (auto s = source_.lock())
        return s->GetGroupId();
    return 0u;
}

bool AreaOfEffect::IsEnemy(const Actor* other) const
{
    if (auto s = source_.lock())
        return s->IsEnemy(other);
    return false;
}

bool AreaOfEffect::IsAlly(const Actor* other) const
{
    if (auto s = source_.lock())
        return s->IsAlly(other);
    return false;
}

bool AreaOfEffect::IsInRange(const Actor* other) const
{
    const float dist = transformation_.position_.Distance(other->GetPosition());
    return dist <= RangeDistances[static_cast<int>(range_)];
}

uint32_t AreaOfEffect::GetItemIndex() const
{
    return itemIndex_;
}

bool AreaOfEffect::Serialize(sa::PropWriteStream& stream)
{
    using namespace AB::GameProtocol;
    static constexpr uint32_t validFields = ObjectSpawnDataFieldName | ObjectSpawnDataFieldModelIndex;
    stream.Write<uint32_t>(validFields);

    if (!GameObject::Serialize(stream))
        return false;
    stream.Write<uint32_t>(GetItemIndex());   // Model/Item index
    return true;
}

void AreaOfEffect::WriteSpawnData(Net::NetworkMessage& msg)
{
    GameObject::WriteSpawnData(msg);

    using namespace AB::GameProtocol;
    static constexpr uint32_t validFields = ObjectSpawnFieldPos | ObjectSpawnFieldRot | ObjectSpawnFieldScale |
        ObjectSpawnFieldUndestroyable | ObjectSpawnFieldSelectable | ObjectSpawnFieldState |
        ObjectSpawnFieldGroupId;
    msg.Add<uint32_t>(validFields);

    msg.Add<float>(transformation_.position_.x_);
    msg.Add<float>(transformation_.position_.y_);
    msg.Add<float>(transformation_.position_.z_);
    msg.Add<float>(transformation_.GetYRotation());
    msg.Add<float>(transformation_.scale_.x_);
    msg.Add<float>(transformation_.scale_.y_);
    msg.Add<float>(transformation_.scale_.z_);
    msg.Add<uint8_t>(1);                                  // not destroyable
    msg.Add<bool>(selectable_);
    msg.Add<uint8_t>(static_cast<uint8_t>(stateComp_.GetState()));
    msg.Add<uint32_t>(GetGroupId());                      // Group id
    sa::PropWriteStream data;
    size_t dataSize;
    Serialize(data);
    const char* cData = data.GetStream(dataSize);
    msg.Add(std::string(cData, dataSize));
}

}
