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

#include "GameObject.h"
#include "Game.h"
#include <AB/ProtocolCodes.h>

void GameObject::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        .addFunction("GetGame", &GameObject::GetGame)
        .addFunction("GetState", &GameObject::GetState)
    );
    // clang-format on
}

GameObject::GameObject(Type type, uint32_t id) : type_(type), id_(id)
{
}

void GameObject::Update(uint32_t timeElapsed)
{
    (void)timeElapsed;
}

Game* GameObject::GetGame() const
{
    return game_;
}

void GameObject::SetGame(Game* game)
{
    game_ = game;
}

void GameObject::SetData(sa::PropReadStream& data)
{
    using namespace AB::GameProtocol;

    uint32_t validFields;
    if (!data.Read<uint32_t>(validFields))
        return;

    if (validFields & ObjectSpawnDataFieldName)
    {
        std::string str;
        if (data.ReadString(str))
            name_ = str;
    }

    if (validFields & ObjectSpawnDataFieldLevel)
        data.Read(level_);
    if (validFields & ObjectSpawnDataFieldPvpCharacter)
    {
        uint8_t isPvp;
        if (data.Read(isPvp))
            pvpCharacter_ = isPvp != 0;
    }

    if (validFields & ObjectSpawnDataFieldSex)
    {
        uint8_t s;
        data.Read(s);
    }

    if (validFields & ObjectSpawnDataFieldProf)
    {
        {
            uint32_t p;
            data.Read(p);
        }
        {
            uint32_t p;
            data.Read(p);
        }
    }
    if (validFields & ObjectSpawnDataFieldModelIndex)
        data.Read(itemIndex_);

    if (validFields & ObjectSpawnDataFieldSkills)
    {
        std::string skills;
        data.ReadString(skills);
    }
}

void GameObject::OnStateChanged(unsigned state)
{
    state_ = state;;
}
