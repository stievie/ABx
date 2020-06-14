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

#include "GameObject.h"
#include <kaguya/kaguya.hpp>
#include <Client.h>
#include <absmath/MathDefs.h>
#include <DirectXMath\DirectXMath.h>

class Player final : public GameObject
{
private:
    Client::Client& client_;
    kaguya::State luaState_;
    void LoadScript(const std::string& script);
    void SelectObject(uint32_t id);
    void FollowObject(uint32_t id);
    void Goto(const Math::StdVector3& pos);
    void Move(unsigned direction);
    void Turn(unsigned direction);
    void SetDirection(float deg);
    void Say(unsigned channel, const std::string& message);
    void Command(unsigned type, const std::string& data);

    void OnStateChanged(unsigned state) override;

public:
    static void RegisterLua(kaguya::State& state);
    Player(Type type, uint32_t id, Client::Client& client, const std::string& script);
    void Update(uint32_t timeElapsed) override;
};
