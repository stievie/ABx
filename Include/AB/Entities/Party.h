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
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_GAMEPARTIES = "game_parties";

struct Party : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GAMEPARTIES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(members, Limits::MAX_PARTY_MEMBERS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
        s.container(npcs, Limits::MAX_PARTY_MEMBERS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_FILENAME);
        });
    }

    // Player UUIDs
    std::vector<std::string> members;
    // NPC scripts
    std::vector<std::string> npcs;
};

}
}
