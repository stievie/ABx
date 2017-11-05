#include "stdafx.h"
#include "IOPlayer.h"

#include "DebugNew.h"

namespace DB {

bool IOPlayer::PreloadPlayer(Game::Player* player, const std::string& name)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    query << "SELECT `id`, `account_id`, `group_id`, `deleted`, (SELECT `type` FROM `accounts` WHERE `accounts`.`id` = `account_id`) AS `account_type`";
    query << " FROM `players` WHERE `name` = " << db->EscapeString(name);
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    if (result->GetULong("deleted") != 0)
        // Was deleted
        return false;

    player->data_.id = result->GetUInt("id");
    player->data_.accountId = result->GetUInt("account_id");

    return true;
}

bool IOPlayer::LoadPlayer(Game::Player* player, std::shared_ptr<DBResult> result)
{
    if (!result)
        return false;

    Database* db = Database::Instance();

    player->data_.accountId = result->GetUInt("account_id");
    player->data_.name = result->GetString("name");
    player->data_.level = result->GetUInt("level");
    player->data_.xp = result->GetUInt("experience");
    player->data_.skillPoints = result->GetUInt("skillpoints");
    player->data_.sex = static_cast<Game::PlayerSex>(result->GetUInt("sex"));

    return true;
}

bool IOPlayer::LoadPlayerByName(Game::Player* player, const std::string & name)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `account_id`, `level`, `experience`, `skillpoints`, " <<
        "`sex`, `lastlogin`, `lastlogout`, `lastip` FROM `players` WHERE `name` = " <<
        db->EscapeString(name);

    return IOPlayer::LoadPlayer(player, db->StoreQuery(query.str()));
}

bool IOPlayer::LoadPlayerById(Game::Player* player, uint32_t playerId)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `account_id`, `level`, `experience`, `skillpoints`, " <<
        "`sex`, `lastlogin`, `lastlogout`, `lastip` FROM `players` WHERE `id` = " << playerId;

    return IOPlayer::LoadPlayer(player, db->StoreQuery(query.str()));
}

}