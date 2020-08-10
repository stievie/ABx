/**
 * Copyright 2020 Stefan Ascher
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

#include "Player.h"
#include "stdafx.h"
#include "BotClient.h"
#include "ScriptHelper.h"
#include <absmath/MathUtils.h>

void Player::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Player"].setClass(kaguya::UserdataMetatable<Player, GameObject>()
        .addFunction("SelectObject", &Player::SelectObject)
        .addFunction("Goto", &Player::Goto)
        .addFunction("Move", &Player::Move)
        .addFunction("Turn", &Player::Turn)
        .addFunction("SetDirection", &Player::SetDirection)
        .addFunction("Say", &Player::Say)
        .addFunction("Command", &Player::Command)
        .addFunction("Interact", &Player::Interact)
        .addFunction("Cancel", &Player::Cancel)
        .addFunction("ChangeMap", &Player::ChangeMap)
        .addFunction("ClickObject", &Player::ClickObject)
        .addFunction("LoadSkills", &Player::LoadSkills)
        .addFunction("UseSkill", &Player::UseSkill)
    );
    // clang-format on
}

Player::Player(Type type, uint32_t id, Client::Client& client, const std::string& script) :
    GameObject(type, id),
    client_(client)
{
    InitLuaState(luaState_);
    luaState_["self"] = this;
    if (!script.empty())
        LoadScript(script);
}

void Player::LoadScript(const std::string& script)
{
    if (!luaState_.dofile(script.c_str()))
    {
        LOG_ERROR << "Failed to execute Lua script " << script << std::endl;
        return;
    }
    if (IsFunction(luaState_, "onInit"))
        luaState_["onInit"]();
}

void Player::Update(uint32_t timeElapsed)
{
    GameObject::Update(timeElapsed);
    if (firstUpdate_)
    {
        if (IsFunction(luaState_, "onEnter"))
            luaState_["onEnter"]();
        firstUpdate_ = false;
    }
    if (IsFunction(luaState_, "onUpdate"))
        luaState_["onUpdate"](timeElapsed);
}

void Player::SelectObject(GameObject* object)
{
    if (object)
        client_.SelectObject(id_, object->id_);
}

void Player::Goto(const Math::StdVector3& pos)
{
    client_.GotoPos({ pos[0], pos[1], pos[2] });
}

void Player::Move(unsigned direction)
{
    client_.Move(static_cast<uint8_t>(direction));
}

void Player::Turn(unsigned direction)
{
    client_.Turn(static_cast<uint8_t>(direction));
}

void Player::SetDirection(float deg)
{
    client_.SetDirection(Math::DegToRad(deg));
}

void Player::Say(unsigned channel, const std::string& message)
{
    AB::GameProtocol::CommandType cmd = AB::GameProtocol::CommandType::Unknown;
    switch (channel)
    {
    case 1:
        cmd = AB::GameProtocol::CommandType::ChatGeneral;
        break;
    case 2:
        cmd = AB::GameProtocol::CommandType::ChatGuild;
        break;
    case 3:
        cmd = AB::GameProtocol::CommandType::ChatParty;
        break;
    case 4:
        cmd = AB::GameProtocol::CommandType::ChatTrade;
        break;
    case 5:
        cmd = AB::GameProtocol::CommandType::ChatWhisper;
        break;
    default:
        return;
    }
    Command(static_cast<unsigned>(cmd), message);
}

void Player::Command(unsigned type, const std::string& data)
{
    client_.Command(static_cast<AB::GameProtocol::CommandType>(type), data);
}

void Player::Cancel()
{
    client_.Cancel();
}

void Player::Interact()
{
    client_.Interact(false, false);
}

void Player::ChangeMap(const std::string& mapUuid)
{
    // Like clicking on an outpost on the maps window
    client_.ChangeMap(mapUuid);
}

void Player::ClickObject(GameObject* object)
{
    if (object)
        client_.ClickObject(id_, object->id_);
}

void Player::LoadSkills(const std::string& templ)
{
    client_.LoadSkillTemplate(templ);
}

void Player::UseSkill(int pos)
{
    client_.UseSkill(false, pos, false);
}

void Player::OnStateChanged(unsigned state)
{
    GameObject::OnStateChanged(state);
    if (IsFunction(luaState_, "onStateChanged"))
        luaState_["onStateChanged"](state);
}

void Player::OnDamaged(GameObject* source)
{
    if (IsFunction(luaState_, "onDamaged"))
        luaState_["onDamaged"](source);
}

void Player::OnHealed(GameObject* source)
{
    if (IsFunction(luaState_, "onHealed"))
        luaState_["onHealed"](source);
}

void Player::OnRecourceChanged(uint8_t type, uint16_t value)
{
    if (IsFunction(luaState_, "onResourceChanged"))
        luaState_["onResourceChanged"](type, value);
}

void Player::ObjectSpawn(GameObject* object)
{
    if (IsFunction(luaState_, "onObjectSpawn"))
        luaState_["onObjectSpawn"](object);
}

void Player::ObjectDespawn(GameObject* object)
{
    if (IsFunction(luaState_, "onObjectDespawn"))
        luaState_["onObjectDespawn"](object);
}
