#include "stdafx.h"
#include "Player.h"

#include "DebugNew.h"

namespace Game {

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    client_(std::move(client))
{
}

Player::~Player()
{
}

void Player::RegisterLua(kaguya::State& state)
{
    Creature::RegisterLua(state);
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Creature>()
        /*        .addFunction("GetName", &Skill::GetName)
        .addFunction("SetName", &Skill::SetName)
        .addFunction("GetDescription", &Skill::GetDescription)
        .addFunction("SetDescription", &Skill::SetDescription)
        .addFunction("GetCooldownTime", &Skill::GetCooldownTime)
        .addFunction("SetCooldownTime", &Skill::SetCooldownTime)*/
    );
}

}
