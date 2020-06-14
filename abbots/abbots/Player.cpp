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

void Player::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Player"].setClass(kaguya::UserdataMetatable<Player, GameObject>()
        .addFunction("SelectObject", &Player::SelectObject)
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
    if (IsFunction(luaState_, "onUpdate"))
        luaState_["onUpdate"]();
}

void Player::SelectObject(uint32_t id)
{
    client_.SelectObject(id_, id);
}
