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

#include "stdafx.h"
#include "DBCharacter.h"

namespace DB {

// Player names are case insensitive. The DB needs a proper index for that:
// CREATE INDEX players_name_ci_index ON players USING btree (lower(name))

bool DBCharacter::Create(AB::Entities::Character& character)
{
    if (Utils::Uuid::IsEmpty(character.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `players` (`uuid`, `profession`, `profession2`, `profession_uuid`, `profession2_uuid`, `name`, `pvp`, " <<
        "`account_uuid`, `level`, `experience`, `skillpoints`, `sex`, " <<
        "`model_index`, `creation`, `inventory_size`";
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
    query << character.creation << ", ";
    query << static_cast<int>(character.inventory_size);

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
    if (!Utils::Uuid::IsEmpty(character.uuid))
        query << "`uuid` = " << db->EscapeString(character.uuid);
    else if (!character.name.empty())
        query << "LOWER(`name`) = LOWER(" << db->EscapeString(character.name) << ")";
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
    character.lastOutpostUuid = result->GetString("last_outpost_uuid");
    character.inventory_size = static_cast<uint16_t>(result->GetUInt("inventory_size"));

    return true;
}

bool DBCharacter::Save(const AB::Entities::Character& character)
{
    if (Utils::Uuid::IsEmpty(character.uuid))
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
    query << " `current_map_uuid` = " << db->EscapeString(character.currentMapUuid) << ", ";
    query << " `last_outpost_uuid` = " << db->EscapeString(character.lastOutpostUuid) << ", ";
    query << " `inventory_size` = " << static_cast<int>(character.inventory_size);

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
    if (Utils::Uuid::IsEmpty(character.uuid))
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
    if (!Utils::Uuid::IsEmpty(character.uuid))
        query << "`uuid` = " << db->EscapeString(character.uuid);
    else if (!character.name.empty())
        query << "LOWER(`name`) = LOWER(" << db->EscapeString(character.name) << ")";
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
