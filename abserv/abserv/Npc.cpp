#include "stdafx.h"
#include "Npc.h"
#include "GameManager.h"

namespace Game {

void Npc::InitializeLua()
{
    GameManager::RegisterLuaAll(luaState_);
    luaState_["this"] = this;
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

    return true;
}

void Npc::Update(uint32_t timeElapsed)
{
    Creature::Update(timeElapsed);
    luaState_["onUpdate"](this, timeElapsed);
}

}
