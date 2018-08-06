#include "stdafx.h"
#include "Npc.h"
#include "GameManager.h"
#include "MathUtils.h"

namespace Game {

void Npc::InitializeLua()
{
    GameManager::RegisterLuaAll(luaState_);
    luaState_["this"] = this;
}

void Npc::RegisterLua(kaguya::State& state)
{
    state["Npc"].setClass(kaguya::UserdataMetatable<Npc, Creature>()
        .addFunction("SetPosition", &Npc::_LuaSetPosition)
        .addFunction("SetRotation", &Npc::_LuaSetRotation)
        .addFunction("SetScale", &Npc::_LuaSetScale)
        .addFunction("GetPosition", &Npc::_LuaGetPosition)
        .addFunction("GetRotation", &Npc::_LuaGetRotation)
        .addFunction("GetScale", &Npc::_LuaGetScale)
    );
}

Npc::Npc() :
    Creature()
{
    InitializeLua();
}

Npc::~Npc()
{
}

bool Npc::LoadScript(const std::string& fileName)
{
    if (!luaState_.dofile(fileName.c_str()))
        return false;

    name_ = (const char*)luaState_["name"];
    level_ = luaState_["level"];
    modelIndex_ = luaState_["modelIndex"];
    sex_ = luaState_["sex"];
    creatureState_ = luaState_["creatureState"];

    IO::DataClient* client = Application::Instance->GetDataClient();

    skills_.prof1_.index = luaState_["prof1Index"];
    if (!client->Read(skills_.prof1_))
    {
        LOG_WARNING << "Unable to read primary profession, index = " << skills_.prof1_.index << std::endl;
    }
    skills_.prof2_.index = luaState_["prof1Index"];
    if (!client->Read(skills_.prof2_))
    {
        LOG_WARNING << "Unable to read secondary profession, index = " << skills_.prof2_.index << std::endl;
    }

    return luaState_["onInit"](this);
}

void Npc::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    Creature::Update(timeElapsed, message);
    luaState_["onUpdate"](this, timeElapsed);
}

void Npc::OnSelected(std::shared_ptr<Creature> selector)
{
    Creature::OnSelected(selector);
    luaState_["onSelected"](this, selector);
}

void Npc::_LuaSetPosition(float x, float y, float z)
{
    transformation_.position_.x_ = x;
    transformation_.position_.y_ = y;
    transformation_.position_.z_ = z;
}

void Npc::_LuaSetRotation(float y)
{
    transformation_.rotation_ = Math::DegToRad(y);
}

void Npc::_LuaSetScale(float x, float y, float z)
{
    transformation_.scale_.x_ = x;
    transformation_.scale_.y_ = y;
    transformation_.scale_.z_ = z;
}

std::vector<float> Npc::_LuaGetPosition() const
{
    std::vector<float> result;
    result.push_back(transformation_.position_.x_);
    result.push_back(transformation_.position_.y_);
    result.push_back(transformation_.position_.z_);
    return result;
}

float Npc::_LuaGetRotation() const
{
    return transformation_.rotation_;
}

std::vector<float> Npc::_LuaGetScale() const
{
    std::vector<float> result;
    result.push_back(transformation_.scale_.x_);
    result.push_back(transformation_.scale_.y_);
    result.push_back(transformation_.scale_.z_);
    return result;
}

}
