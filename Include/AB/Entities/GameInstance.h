/**
 * Copyright 2017-2020 Stefan Ascher
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

#include <AB/Entities/Entity.h>
#include <AB/Entities/Game.h>

namespace AB {
namespace Entities {

struct GameInstance : Entity
{
    MAKE_ENTITY(GameInstance)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(gameUuid, Limits::MAX_UUID);
        s.text1b(serverUuid, Limits::MAX_UUID);
        s.text1b(name, Limits::MAX_GAME_INSTANCE_NAME);
        s.text1b(recording, Limits::MAX_FILENAME);
        s.value8b(startTime);
        s.value8b(stopTime);
        s.value2b(number);
        s.value1b(running);
        s.value2b(players);
    }

    std::string gameUuid = EMPTY_GUID;
    /// Server running this instance
    std::string serverUuid = EMPTY_GUID;
    std::string name;
    /// Recording filename
    std::string recording;
    timestamp_t startTime{ 0 };
    timestamp_t stopTime{ 0 };
    /// If there are more instances of the same game on the same server this is the number, e.g. District.
    uint16_t number = 0;
    bool running{ false };
    // Number of players in this instance
    uint16_t players{ 0 };
};

}
}
