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

#pragma once

#include <absmath/Transformation.h>
#include <kaguya/kaguya.hpp>
#include <sa/PropStream.h>

class Game;

class GameObject
{
private:
    Game* game_{ nullptr };
public:
    enum class Type
    {
        ItemDrop,
        AOE,
        Projectile,
        Npc,
        Player,
        Self,
    };
    static void RegisterLua(kaguya::State& state);

    GameObject(Type type, uint32_t id);

    virtual void Update(uint32_t timeElapsed);
    Game* GetGame() const;
    unsigned GetState() const { return state_; }
    void SetGame(Game* game);
    void SetData(sa::PropReadStream& data);

    virtual void OnStateChanged(unsigned);

    Type type_;
    uint32_t id_{ 0 };
    Math::Transformation transformation_;
    uint32_t level_{ 0 };
    bool pvpCharacter_{ false };
    uint32_t itemIndex_{ 0 };
    std::string name_;
    unsigned state_{ 0 };
};
