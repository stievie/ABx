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

namespace DB {

bool DBQuest::Create(AB::Entities::Quest& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `game_quests` (`uuid`, `idx`, `name`, `script`, `repeatable`, `description` " <<
        "`depends_on_uuid`, `reward_xp`, `reward_money`, `reward_items`";
    query << ") VALUES (";

    query << db->EscapeString(v.uuid) << ", ";
    query << v.index << ", ";
    query << db->EscapeString(v.name) << ", ";
    query << db->EscapeString(v.script) << ", ";
    query << (v.repeatable ? 1 : 0) << ", ";
    query << db->EscapeString(v.description) << ", ";
    query << db->EscapeString(v.dependsOn) << ", ";
    query << v.rewardXp << ", ";
    query << v.rewardMoney << ", ";
    query << db->EscapeString(sa::CombineString(v.rewardItems, std::string(";")));

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

bool DBQuest::Load(AB::Entities::Quest& v)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `game_quests` WHERE ";
    if (!Utils::Uuid::IsEmpty(v.uuid))
        query << "`uuid` = " << db->EscapeString(v.uuid);
    else if (v.index != AB::Entities::INVALID_INDEX)
        query << "idx = " << v.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
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
    v.rewardItems = Utils::Split(result->GetString("reward_items"), ";");

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
    std::ostringstream query;

    query << "UPDATE `game_quests` SET ";

    // Only these may be changed
    query << " `name` = " << db->EscapeString(v.name) << ", ";
    query << " `script` = " << db->EscapeString(v.script) << ", ";
    query << " `repeatable` = " << (v.repeatable ? 1 : 0) << ", ";
    query << " `description` = " << db->EscapeString(v.description) << ", ";
    query << " `depends_on_uuid` = " << db->EscapeString(v.dependsOn) << ", ";
    query << " `reward_xp` = " << v.rewardXp << ", ";
    query << " `reward_money` = " << v.rewardMoney << ", ";
    query << " `reward_items` = " << sa::CombineString(v.rewardItems, std::string(";"));

    query << " WHERE `uuid` = " << db->EscapeString(v.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
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

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_quests` WHERE ";
    if (!Utils::Uuid::IsEmpty(v.uuid))
        query << "`uuid` = " << db->EscapeString(v.uuid);
    else if (v.index != AB::Entities::INVALID_INDEX)
        query << "idx = " << v.index << ")";
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
