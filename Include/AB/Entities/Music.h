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

namespace AB {
namespace Entities {

enum MusicStyle : uint32_t
{
    MusicStyleUnknown = 0,
    MusicStyleEastern = 1 << 1,
    MusicSttyleWestern = 1 << 2,
    MusicStyleSlow = 1 << 3,
    MusicStyleDriving = 1 << 4,
    MusicStyleEpic = 1 << 5,
    MusicStyleAction = 1 << 6,
    MusicStyleMystic = 1 << 7,
    MusicStyleIntense = 1 << 8,
    MusicStyleAggressive = 1 << 9,
    MusicStyleRelaxed = 1 << 10,
    MusicStyleHumorous = 1 << 11
};

struct Music : Entity
{
    MAKE_ENTITY(Music)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(mapUuid, Limits::MAX_MUSIC_MAPS);
        s.text1b(localFile, Limits::MAX_FILENAME);
        s.text1b(remoteFile, Limits::MAX_FILENAME);
        s.value1b(sorting);
        s.value4b(style);
    }

    std::string mapUuid;
    std::string localFile;
    std::string remoteFile;
    uint8_t sorting = 0;
    MusicStyle style = MusicStyleUnknown;
};

}
}
