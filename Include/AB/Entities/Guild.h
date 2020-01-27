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

static constexpr auto KEY_GUILD = "guilds";

struct Guild : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GUILD;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_GUILD_NAME);
        s.text1b(tag, Limits::MAX_GUILD_TAG);
        s.text1b(creatorAccountUuid, Limits::MAX_UUID);
        s.text1b(creatorName, Limits::MAX_CHARACTER_NAME);
        s.text1b(creatorPlayerUuid, Limits::MAX_UUID);
        s.value8b(creation);
        s.text1b(guildHall, Limits::MAX_UUID);
        s.text1b(guildHallInstanceUuid, Limits::MAX_UUID);
        s.text1b(guildHallServerUuid, Limits::MAX_UUID);
    }

    std::string name;
    std::string tag;
    std::string creatorAccountUuid = EMPTY_GUID;
    std::string creatorName;
    std::string creatorPlayerUuid = EMPTY_GUID;
    timestamp_t creation;
    std::string guildHall = EMPTY_GUID;
    std::string guildHallInstanceUuid = EMPTY_GUID;
    std::string guildHallServerUuid = EMPTY_GUID;
};

}
}
