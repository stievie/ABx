#include "stdafx.h"
#include "AreaOfEffect.h"
#include "Subsystems.h"
#include "DataProvider.h"
#include "ScriptManager.h"
#include "Actor.h"
#include "Game.h"
#include "Mechanic.h"

namespace Game {

void AreaOfEffect::InitializeLua()
{
    ScriptManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
    luaInitialized_ = true;
}

void AreaOfEffect::_LuaSetSource(Actor* source)
{
    if (source)
        source_ = source->GetThis<Actor>();
    else
        source_.reset();
}

Actor* AreaOfEffect::_LuaGetSource()
{
    auto source = GetSource();
    if (source)
        return source.get();
    return nullptr;
}

void AreaOfEffect::RegisterLua(kaguya::State& state)
{
    state["AreaOfEffect"].setClass(kaguya::UserdataMetatable<AreaOfEffect, GameObject>()
        .addFunction("SetRange", &AreaOfEffect::SetRange)
        .addFunction("GetRange", &AreaOfEffect::GetRange)
        .addFunction("GetSource", &AreaOfEffect::_LuaGetSource)
        .addFunction("SetSource", &AreaOfEffect::_LuaSetSource)
        .addFunction("GetStartTime", &AreaOfEffect::GetStartTime)
        .addFunction("GetLifetime", &AreaOfEffect::GetLifetime)
        .addFunction("SetLifetime", &AreaOfEffect::SetLifetime)
        .addFunction("Index", &AreaOfEffect::GetIndex)
    );
}

AreaOfEffect::AreaOfEffect() :
    GameObject(),
    luaInitialized_(false),
    index_(0),
    range_(Ranges::Aggro),
    functions_(FunctionNone),
    startTime_(Utils::Tick()),
    lifetime_(std::numeric_limits<uint32_t>::max())
{
    SetCollisionShape(
        std::make_unique<Math::CollisionShapeImpl<Math::Sphere>>(Math::ShapeTypeSphere,
            Math::Vector3::Zero, RANGE_ADJECENT)
    );

    InitializeLua();
}

bool AreaOfEffect::LoadScript(const std::string& fileName)
{
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;
    if (ScriptManager::IsFunction(luaState_, "onUpdate"))
        functions_ |= FunctionUpdate;
    if (ScriptManager::IsFunction(luaState_, "onEnded"))
        functions_ |= FunctionEnded;
    if (ScriptManager::IsFunction(luaState_, "onTrigger"))
        functions_ |= FunctionOnTrigger;

    bool ret = luaState_["onInit"]();
    return ret;
}

void AreaOfEffect::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);

    if (luaInitialized_ && HaveFunction(FunctionUpdate))
        ScriptManager::CallFunction(luaState_, "onUpdate", timeElapsed);
    if (Utils::TimePassed(startTime_) > lifetime_)
    {
        if (luaInitialized_ && HaveFunction(FunctionEnded))
            ScriptManager::CallFunction(luaState_, "onEnded");
        GetGame()->RemoveObject(this);
    }
}

void AreaOfEffect::OnTrigger(GameObject* other)
{
    // Called from triggerComp_
    GameObject::OnTrigger(other);

    // AOE can also be a trap for example
    if (luaInitialized_ && HaveFunction(FunctionOnTrigger))
        ScriptManager::CallFunction(luaState_, "onTrigger", other);
}

void AreaOfEffect::SetRange(Ranges range)
{
    assert(static_cast<int>(range) >= 0 && static_cast<int>(range) <= static_cast<int>(Ranges::Map));

    range_ = range;

    // Update collision shape size
    using SphereShape = Math::CollisionShapeImpl<Math::Sphere>;
    auto cs = GetCollisionShape();
    if (!cs)
        return;
    if (cs->shapeType_ != Math::ShapeTypeSphere)
        // AOE should always have a sphere
        return;

    SphereShape* shape = static_cast<SphereShape*>(cs);
    shape->Object()->radius_ = RangeDistances[static_cast<int>(range)];
}

void AreaOfEffect::SetSource(std::shared_ptr<Actor> source)
{
    source_ = source;
}

std::shared_ptr<Actor> AreaOfEffect::GetSource()
{
    return source_.lock();
}

bool AreaOfEffect::Serialize(IO::PropWriteStream& stream)
{
    if (!GameObject::Serialize(stream))
        return false;
    return true;
}

void AreaOfEffect::WriteSpawnData(Net::NetworkMessage& msg)
{
    msg.Add<uint32_t>(id_);
    msg.Add<float>(transformation_.position_.x_);
    msg.Add<float>(transformation_.position_.y_);
    msg.Add<float>(transformation_.position_.z_);
    msg.Add<float>(transformation_.GetYRotation());
    msg.Add<float>(transformation_.scale_.x_);
    msg.Add<float>(transformation_.scale_.y_);
    msg.Add<float>(transformation_.scale_.z_);
    msg.Add<uint8_t>(1);                                  // destroyable
    msg.Add<uint8_t>(stateComp_.GetState());
    msg.Add<float>(0.0f);                                 // speed
    msg.Add<uint32_t>(0);                                 // Group id
    msg.Add<uint8_t>(0);                                  // Group pos
    IO::PropWriteStream data;
    size_t dataSize;
    Serialize(data);
    const char* cData = data.GetStream(dataSize);
    msg.AddString(std::string(cData, dataSize));
}

}
