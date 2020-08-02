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

#include "DBQuest.h"
#include <sa/StringTempl.h>
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Quest& q, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(q.uuid);
        if (token.value == "idx")
            return std::to_string(q.index);
        if (token.value == "name")
            return db->EscapeString(q.name);
        if (token.value == "script")
            return db->EscapeString(q.script);
        if (token.value == "repeatable")
            return std::to_string(q.repeatable ? 1 : 0);
        if (token.value == "description")
            return db->EscapeString(q.description);
        if (token.value == "depends_on_uuid")
            return db->EscapeString(q.dependsOn);
        if (token.value == "reward_xp")
            return std::to_string(q.rewardXp);
        if (token.value == "reward_money")
            return std::to_string(q.rewardMoney);
        if (token.value == "reward_items")
            return db->EscapeString(sa::CombineString<char>(q.rewardItems, std::string(";")));

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBQuest::Create(AB::Entities::Quest& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "INSERT INTO game_quests ("
            "uuid, idx, name, script, repeatable, description, depends_on_uuid, reward_xp, reward_money, reward_items"
        ") VALUES ("
            "${uuid}, ${idx}, ${name}, ${script}, ${repeatable}, ${description}, ${depends_on_uuid}, ${reward_xp}, ${reward_money}, ${reward_items}"
        ")";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBQuest::Load(AB::Entities::Quest& v)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT * FROM game_quests WHERE uuid= ${uuid}";
    static constexpr const char* SQL_INDEX = "SELECT * FROM game_quests WHERE idx = ${index}";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(v.uuid))
        sql = SQL_UUID;
    else if (!v.index != 0)
        sql = SQL_INDEX;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    v.uuid = result->GetString("uuid");
    v.index = result->GetUInt("idx");
    v.name = result->GetString("name");
    v.script = result->GetString("script");
    v.repeatable = result->GetUInt("repeatable") != 0;
    v.description = result->GetString("description");
    v.dependsOn = result->GetString("depends_on_uuid");
    v.rewardXp = result->GetInt("reward_xp");
    v.rewardMoney = result->GetInt("reward_money");
    v.rewardItems = sa::Split(result->GetString("reward_items"), ";");

    return true;
}

bool DBQuest::Save(const AB::Entities::Quest& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    // Only these may be changed
    static constexpr const char* SQL = "UPDATE game_quests SET "
        "name = ${name}, "
        "script = ${script}, "
        "repeatable = ${repeatable}, "
        "description = ${description}, "
        "depends_on_uuid = ${depends_on_uuid}, "
        "reward_xp = ${reward_xp}, "
        "reward_money = ${reward_money}, "
        "reward_items = ${reward_items} "
        "WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBQuest::Delete(const AB::Entities::Quest&)
{
    // Do nothing
    return true;
}

bool DBQuest::Exists(const AB::Entities::Quest& v)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT COUNT(*) AS count FROM game_quests WHERE uuid= ${uuid}";
    static constexpr const char* SQL_INDEX = "SELECT COUNT(*) AS count FROM game_quests WHERE idx = ${index}";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(v.uuid))
        sql = SQL_UUID;
    else if (!v.index != 0)
        sql = SQL_INDEX;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
