#include "stdafx.h"
#include "Npc.h"
#include "GameManager.h"
#include "MathUtils.h"

namespace Game {

void Npc::InitializeLua()
{
    Creature::InitializeLua();
}

void Npc::RegisterLua(kaguya::State& state)
{
    state["Npc"].setClass(kaguya::UserdataMetatable<Npc, Creature>()
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

    return luaState_["onInit"]();
}

void Npc::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    Creature::Update(timeElapsed, message);
    luaState_["onUpdate"](timeElapsed);
}

}
