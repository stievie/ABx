#include "stdafx.h"
#include "IOPlayer.h"
#include "PropStream.h"
#include "IOGame.h"
#include "Utils.h"
#include "ConfigManager.h"

#include "DebugNew.h"

namespace DB {

bool IOPlayer::PreloadPlayer(Game::Player* player, const std::string& name)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    query << "SELECT `id`, `account_id`, `deleted`, (SELECT `type` FROM `accounts` WHERE `accounts`.`id` = `account_id`) AS `account_type`";
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

    player->data_.accountId = result->GetUInt("account_id");
    player->data_.name = result->GetString("name");
    player->data_.pvp = result->GetUInt("pvp") != 0;
    player->data_.level = static_cast<uint16_t>(result->GetUInt("level"));
    player->data_.xp = result->GetULong("experience");
    player->data_.skillPoints = result->GetUInt("skillpoints");
    player->data_.sex = static_cast<AB::Data::CreatureSex>(result->GetUInt("sex"));
    if (!result->IsNull("last_map"))
        player->data_.lastMap = result->GetString("last_map");
    else
        player->data_.lastMap = IOGame::GetLandingGame();

    return true;
}

bool IOPlayer::LoadPlayerByName(Game::Player* player, const std::string& name)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `pvp`, `account_id`, `level`, `experience`, `skillpoints`, " <<
        "`sex` FROM `players` WHERE `name` = " <<
        db->EscapeString(name);

    return IOPlayer::LoadPlayer(player, db->StoreQuery(query.str()));
}

bool IOPlayer::LoadPlayerById(Game::Player* player, uint32_t playerId)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `pvp`, `account_id`, `level`, `experience`, `skillpoints`, " <<
        "`sex` FROM `players` WHERE `id` = " << playerId;

    return IOPlayer::LoadPlayer(player, db->StoreQuery(query.str()));
}

bool IOPlayer::SavePlayer(Game::Player* player)
{
    Database* db = Database::Instance();

    // Conditions
    IO::PropWriteStream stream;
    for (auto& effect : player->effects_)
    {
        if (effect->IsPersistent())
        {
            effect->Serialize(stream);
            stream.Write<uint8_t>(Game::EffectAttrEnd);
        }
    }
    size_t effectSize;
    const char* effectBlob = stream.GetStream(effectSize);

    std::ostringstream query;
    query << "UPDATE `players` SET ";
    query << " `level` = " << player->data_.level << ",";
    query << " `experience` = " << player->data_.xp << ",";
    query << " `skillpoints` = " << player->data_.skillPoints << ",";
    query << " `lastlogin` = " << player->loginTime_ << ",";
    query << " `lastlogout` = " << player->logoutTime_ << ",";
    query << " `lastip` = " << player->client_->GetIP() << ",";
    // Online time in sec
    query << " `onlinetime` = `onlinetime` + " << static_cast<int>((player->logoutTime_ - player->loginTime_) / 1000) << ",";

    query << " `effects` = " << db->EscapeBlob(effectBlob, static_cast<uint32_t>(effectSize)) << ",";

    query << " `last_map` = " << db->EscapeString(player->data_.lastMap);

    query << " WHERE `id` = " << player->data_.id;

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

IOPlayer::CreatePlayerResult IOPlayer::CreatePlayer(uint32_t accountId,
    std::string& name, const std::string& prof, AB::Data::CreatureSex sex, bool isPvp)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    std::shared_ptr<DBResult> result;
    query << "SELECT COUNT(`id`) AS `c`, `char_slots` FROM `accounts` WHERE `id` = " << accountId;
    result = db->StoreQuery(query.str());
    if (!result)
        return ResultInternalError;
    if (result->GetUInt("c") != 1)
        return ResultInvalidAccount;
    uint32_t charSlots = result->GetUInt("char_slots");

    query.str("");
    query << "SELECT COUNT(`id`) AS `c` FROM `players` WHERE `account_id` = " << accountId;
    result = db->StoreQuery(query.str());
    if (!result)
        return ResultInternalError;
    if (result->GetUInt("c") >= charSlots)
        return ResultNoMoreCharSlots;

    query.str("");
    query << "SELECT COUNT(`id`) AS `c` FROM `players` WHERE `name` = " << db->EscapeString(name);
    result = db->StoreQuery(query.str());
    if (!result)
        return ResultInternalError;
    if (result->GetUInt("c") != 0)
        return ResultNameExists;

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return ResultInternalError;

    unsigned level = 1;
    if (isPvp)
    {
        level = ConfigManager::Instance[ConfigManager::Key::PlayerLevelCap].GetInt();
    }

    query.str("");
    query << "INSERT INTO `players` (`profession`, `name`, `sex`, `pvp`, `account_id`, `level`, `creation`) VALUES (";
    query << db->EscapeString(prof) << ", ";
    query << db->EscapeString(name) << ", ";
    query << static_cast<uint32_t>(sex) << ", ";
    query << (isPvp ? 1 : 0) << ", ";
    query << accountId << ", ";
    query << level << ", ";
    query << Utils::AbTick();
    query << ")";
    if (!db->ExecuteQuery(query.str()))
        return ResultInternalError;

    // End transaction
    if (!transaction.Commit())
        return ResultInternalError;

    return ResultOK;
}

bool IOPlayer::DeletePlayer(uint32_t accountId, uint32_t playerId)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    std::shared_ptr<DBResult> result;
    query << "SELECT COUNT(`id`) AS `c` FROM `players` WHERE `account_id` = " << accountId;
    query << " AND `id` = " << playerId;
    result = db->StoreQuery(query.str());
    if (!result)
        return false;
    if (result->GetUInt("c") != 1)
        return false;

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    query.str("");
    query << "DELETE FROM `players` WHERE `id` = " << playerId;
    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

}