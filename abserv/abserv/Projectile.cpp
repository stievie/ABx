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
        .addFunction("GetItem", &Projectile::GetItem)
    );
}

Projectile::Projectile(std::unique_ptr<Item>& item) :
    Actor(),
    item_(std::move(item)),
    luaInitialized_(false),
    startSet_(false)
{
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
        start_ = source->transformation_.position_ + HeadOffset;
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

    Actor::Update(timeElapsed, message);

    if (luaInitialized_ && HaveFunction(FunctionUpdate))
        ScriptManager::CallFunction(luaState_, "onUpdate", timeElapsed);
    if (item_)
        item_->Update(timeElapsed);
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
    if (luaInitialized_ && HaveFunction(FunctionOnStart))
    {
        bool ret = luaState_["onStart"](t.get());
        return ret;
    }
    return true;
}

Item* Projectile::GetItem() const
{
    if (item_)
        return item_.get();
    return nullptr;
}

bool Projectile::Serialize(IO::PropWriteStream& stream)
{
    if (!Actor::Serialize(stream))
        return false;
    stream.Write<uint32_t>(0);                                   // Level
    stream.Write<uint8_t>(AB::Entities::CharacterSexUnknown);
    stream.Write<uint32_t>(0);                                   // Prof 1
    stream.Write<uint32_t>(0);                                   // Prof 2
    // TODO: Item index for projectiles
    stream.Write<uint32_t>(0);   // Model index, in this case the effect index
    return true;
}

void Projectile::WriteSpawnData(Net::NetworkMessage& msg)
{
    msg.Add<uint32_t>(id_);
    msg.Add<float>(transformation_.position_.x_);
    msg.Add<float>(transformation_.position_.y_);
    msg.Add<float>(transformation_.position_.z_);
    msg.Add<float>(transformation_.GetYRotation());
    msg.Add<float>(transformation_.scale_.x_);
    msg.Add<float>(transformation_.scale_.y_);
    msg.Add<float>(transformation_.scale_.z_);
    msg.Add<uint8_t>(1);                                  // not destroyable
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
