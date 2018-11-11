#include "stdafx.h"
#include "DBCharacter.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBCharacter::Create(AB::Entities::Character& character)
{
    if (character.uuid.empty() || uuids::uuid(character.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `players` (`uuid`, `profession`, `profession2`, `profession_uuid`, `profession2_uuid`, `name`, `pvp`, " <<
        "`account_uuid`, `level`, `experience`, `skillpoints`, `sex`, " <<
        "`model_index`, `creation`";
    query << ") VALUES (";

    query << db->EscapeString(character.uuid) << ", ";
    query << db->EscapeString(character.profession) << ", ";
    query << db->EscapeString(character.profession2) << ", ";
    query << db->EscapeString(character.professionUuid) << ", ";
    query << db->EscapeString(character.profession2Uuid) << ", ";
    query << db->EscapeString(character.name) << ", ";
    query << (character.pvp ? 1 : 0) << ", ";
    query << db->EscapeString(character.accountUuid) << ", ";
    query << static_cast<int>(character.level) << ", ";
    query << character.xp << ", ";
    query << character.skillPoints << ", ";
    query << static_cast<uint32_t>(character.sex) << ", ";
    query << character.modelIndex << ", ";
    query << character.creation;

    query << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBCharacter::Load(AB::Entities::Character& character)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `players` WHERE ";
    if (!character.uuid.empty() && !uuids::uuid(character.uuid).nil())
        query << "`uuid` = " << db->EscapeString(character.uuid);
    else if (!character.name.empty())
        query << "`name` = " << db->EscapeString(character.name);
    else
    {
        LOG_ERROR << "UUID (" << character.uuid << ") and name (" << character.name << ") are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    character.uuid = result->GetString("uuid");
    character.profession = result->GetString("profession");
    character.profession2 = result->GetString("profession2");
    character.professionUuid = result->GetString("profession_uuid");
    character.profession2Uuid = result->GetString("profession2_uuid");
    character.name = result->GetString("name");
    character.pvp = result->GetInt("pvp") != 0;
    character.accountUuid = result->GetString("account_uuid");
    character.skillTemplate = result->GetString("skills");
    character.level = static_cast<uint8_t>(result->GetUInt("level"));
    character.xp = result->GetUInt("experience");
    character.skillPoints = result->GetUInt("skillpoints");
    character.sex = static_cast<AB::Entities::CharacterSex>(result->GetUInt("sex"));
    character.modelIndex = result->GetUInt("model_index");
    character.lastLogin = result->GetLong("lastlogin");
    character.lastLogout = result->GetLong("lastlogout");
    character.creation = result->GetLong("creation");
    character.onlineTime = result->GetLong("onlinetime");
    character.deletedTime = result->GetLong("deleted");
    character.currentMapUuid = result->GetString("current_map_uuid");

    return true;
}

bool DBCharacter::Save(const AB::Entities::Character& character)
{
    if (character.uuid.empty() || uuids::uuid(character.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `players` SET ";

    // Only these may be changed
    query << " `profession2` = " << db->EscapeString(character.profession2) << ", ";
    query << " `profession2_uuid` = " << db->EscapeString(character.profession2Uuid) << ", ";
    query << " `skills` = " << db->EscapeString(character.skillTemplate) << ", ";
    query << " `level` = " << static_cast<int>(character.level) << ", ";
    query << " `experience` = " << character.xp << ", ";
    query << " `skillpoints` = " << character.skillPoints << ", ";
    query << " `lastlogin` = " << character.lastLogin << ", ";
    query << " `lastlogout` = " << character.lastLogout << ", ";
    query << " `onlinetime` = " << character.onlineTime << ", ";
    query << " `deleted` = " << character.deletedTime << ", ";
    query << " `current_map_uuid` = " << db->EscapeString(character.currentMapUuid);

    query << " WHERE `uuid` = " << db->EscapeString(character.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBCharacter::Delete(const AB::Entities::Character& character)
{
    if (character.uuid.empty() || uuids::uuid(character.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `players` WHERE `uuid` = " << db->EscapeString(character.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBCharacter::Exists(const AB::Entities::Character& character)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `players` WHERE ";
    if (!character.uuid.empty() && !uuids::uuid(character.uuid).nil())
        query << "`uuid` = " << db->EscapeString(character.uuid);
    else if (!character.name.empty())
        query << "`name` = " << db->EscapeString(character.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
