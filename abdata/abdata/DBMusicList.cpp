#include "stdafx.h"
#include "DBMusicList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBMusicList::Create(AB::Entities::MusicList&)
{
    return true;
}

bool DBMusicList::Load(AB::Entities::MusicList& il)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `game_music` ORDER BY `sorting`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        il.musicUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBMusicList::Save(const AB::Entities::MusicList&)
{
    return true;
}

bool DBMusicList::Delete(const AB::Entities::MusicList&)
{
    return true;
}

bool DBMusicList::Exists(const AB::Entities::MusicList&)
{
    return true;
}

}
