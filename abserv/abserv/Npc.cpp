#include "stdafx.h"
#include "Npc.h"
#include "GameManager.h"
#include "MathUtils.h"

namespace Game {

void Npc::InitializeLua()
{
    Creature::InitializeLua();
    luaState_["self"] = this;
}

void Npc::RegisterLua(kaguya::State& state)
{
    state["Npc"].setClass(kaguya::UserdataMetatable<Npc, Creature>()
        .addFunction("Say", &Npc::Say)
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
    stateComp_.SetState(luaState_["creatureState"], true);

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

void Npc::Say(ChatType channel, const std::string& message)
{
    switch (channel)
    {
    case ChannelMap:
    {
        std::shared_ptr<ChatChannel> ch = Chat::Instance.Get(ChannelMap, static_cast<uint64_t>(GetGame()->id_));
        if (ch)
        {
            ch->TalkNpc(this, message);
        }
        break;
    }
    case ChannelAllies:
        break;
    case ChannelParty:
        break;
    }
}

}
