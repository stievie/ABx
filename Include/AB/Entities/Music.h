#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_MUSIC = "game_music";

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
    static constexpr const char* KEY()
    {
        return KEY_MUSIC;
    }
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
