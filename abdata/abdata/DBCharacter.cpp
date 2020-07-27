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

#include "DBCharacter.h"
#include <sa/TemplateParser.h>

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
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    static constexpr const char* SQL = "INSERT INTO players ("
            "uuid, profession, profession2, profession_uuid, profession2_uuid, name, pvp, "
            "account_uuid, level, experience, skillpoints, sex, model_index, creation, inventory_size"
        ") VALUES ("
            "${uuid}, ${profession}, ${profession2}, ${profession_uuid}, ${profession2_uuid}, ${name}, ${pvp}, "
            "${account_uuid}, ${level}, ${experience}, ${skillpoints}, ${sex}, ${model_index}, ${creation}, ${inventory_size}"
        ")";

    auto callback = [db, &character](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "uuid")
                return db->EscapeString(character.uuid);
            if (token.value == "profession")
                return db->EscapeString(character.profession);
            if (token.value == "profession2")
                return db->EscapeString(character.profession2);
            if (token.value == "profession_uuid")
                return db->EscapeString(character.professionUuid);
            if (token.value == "profession2_uuid")
                return db->EscapeString(character.profession2Uuid);
            if (token.value == "name")
                return db->EscapeString(character.name);
            if (token.value == "pvp")
                return std::to_string(character.pvp ? 1 : 0);
            if (token.value == "account_uuid")
                return db->EscapeString(character.accountUuid);
            if (token.value == "level")
                return std::to_string(character.level);
            if (token.value == "experience")
                return std::to_string(character.xp);
            if (token.value == "skillpoints")
                return std::to_string(character.skillPoints);
            if (token.value == "sex")
                return std::to_string(static_cast<uint32_t>(character.sex));
            if (token.value == "model_index")
                return std::to_string(character.modelIndex);
            if (token.value == "creation")
                return std::to_string(character.creation);
            if (token.value == "inventory_size")
                return std::to_string(character.inventorySize);

            ASSERT_FALSE();
        default:
            return token.value;
        }
    };

    if (!db->ExecuteQuery(sa::templ::Parser::Evaluate(SQL, callback)))
        return false;

    return transaction.Commit();
}

bool DBCharacter::Load(AB::Entities::Character& character)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT * FROM players WHERE uuid= ${uuid}";
    static constexpr const char* SQL_NAME = "SELECT * FROM accounts WHERE LOWER(name) = LOWER(${name})";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(character.uuid))
        sql = SQL_UUID;
    else if (!character.name.empty())
        sql = SQL_NAME;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    const std::string query = sa::templ::Parser::Evaluate(sql, [db, &character](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "uuid")
                return db->EscapeString(character.uuid);
            if (token.value == "name")
                return db->EscapeString(character.name);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
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
    character.inventorySize = static_cast<uint16_t>(result->GetUInt("inventory_size"));
    character.deathStats = result->GetStream("death_stats");

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
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    static constexpr const char* SQL = "UPDATE players SET "
        "profession2 = ${profession2}, "
        "profession2_uuid = ${profession2_uuid}, "
        "skills = ${skills}, "
        "level = ${level}, "
        "experience = ${experience}, "
        "skillpoints = ${skillpoints}, "
        "lastlogin = ${lastlogin}, "
        "lastlogout = ${lastlogout}, "
        "onlinetime = ${onlinetime}, "
        "deleted = ${deleted}, "
        "current_map_uuid = ${current_map_uuid}, "
        "last_outpost_uuid = ${last_outpost_uuid}, "
        "inventory_size = ${inventory_size}, "
        "death_stats = ${death_stats} "
        "WHERE uuid = ${uuid}";

    auto callback = [db, &character](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "profession2")
                return db->EscapeString(character.profession2);
            if (token.value == "profession2_uuid")
                return db->EscapeString(character.profession2Uuid);
            if (token.value == "skills")
                return db->EscapeString(character.skillTemplate);
            if (token.value == "level")
                return std::to_string(character.level);
            if (token.value == "experience")
                return std::to_string(character.xp);
            if (token.value == "skillpoints")
                return std::to_string(character.skillPoints);
            if (token.value == "lastlogin")
                return std::to_string(character.lastLogin);
            if (token.value == "lastlogout")
                return std::to_string(character.lastLogout);
            if (token.value == "onlinetime")
                return std::to_string(character.onlineTime);
            if (token.value == "deleted")
                return std::to_string(character.deletedTime);
            if (token.value == "current_map_uuid")
                return db->EscapeString(character.currentMapUuid);
            if (token.value == "last_outpost_uuid")
                return db->EscapeString(character.lastOutpostUuid);
            if (token.value == "inventory_size")
                return std::to_string(character.inventorySize);
            if (token.value == "death_stats")
                return db->EscapeBlob(character.deathStats.data(), character.deathStats.length());
            if (token.value == "uuid")
                return db->EscapeString(character.uuid);

            ASSERT_FALSE();
        default:
            return token.value;
        }
    };

    if (!db->ExecuteQuery(sa::templ::Parser::Evaluate(SQL, callback)))
        return false;

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
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    static constexpr const char* SQL = "DELETE FROM players WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, [db, &character](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "uuid")
                return db->EscapeString(character.uuid);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBCharacter::Exists(const AB::Entities::Character& character)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT COUNT(*) AS count FROM players WHERE uuid= ${uuid}";
    static constexpr const char* SQL_NAME = "SELECT COUNT(*) AS count FROM players WHERE LOWER(name) = LOWER(${name})";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(character.uuid))
        sql = SQL_UUID;
    else if (!character.name.empty())
        sql = SQL_NAME;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    const std::string query = sa::templ::Parser::Evaluate(sql, [db, &character](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "uuid")
                return db->EscapeString(character.uuid);
            if (token.value == "name")
                return db->EscapeString(character.name);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
