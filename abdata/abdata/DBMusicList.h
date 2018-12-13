#pragma once

#include <AB/Entities/MusicList.h>

namespace DB {

class DBMusicList
{
public:
    DBMusicList() = delete;
    ~DBMusicList() = delete;

    static bool Create(AB::Entities::MusicList&);
    static bool Load(AB::Entities::MusicList& il);
    static bool Save(const AB::Entities::MusicList&);
    static bool Delete(const AB::Entities::MusicList&);
    static bool Exists(const AB::Entities::MusicList&);
};

}
