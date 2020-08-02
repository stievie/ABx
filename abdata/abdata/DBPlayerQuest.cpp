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

#include "DBPlayerQuest.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::PlayerQuest& q, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(q.uuid);
        if (token.value == "quests_uuid")
            return db->EscapeString(q.questUuid);
        if (token.value == "player_uuid")
            return db->EscapeString(q.playerUuid);
        if (token.value == "completed")
            return std::to_string(q.completed ? 1 : 0);
        if (token.value == "rewarded")
            return std::to_string(q.rewarded ? 1 : 0);
        if (token.value == "progress")
            return db->EscapeBlob(q.progress.data(), q.progress.size());
        if (token.value == "picked_up_times")
            return std::to_string(q.pickupTime);
        if (token.value == "completed_time")
            return std::to_string(q.completeTime);
        if (token.value == "rewarded_time")
            return std::to_string(q.rewardTime);
        if (token.value == "deleted")
            return std::to_string(q.deleted ? 1 : 0);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBPlayerQuest::Create(AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "INSERT INTO player_quests ("
            "uuid, quests_uuid, player_uuid, completed, rewarded, progress, picked_up_times, completed_time, rewarded_time, deleted"
        ") VALUES ("
            "${uuid, ${quests_uuid}, ${player_uuid}, ${completed}, ${rewarded}, ${progress}, ${picked_up_times}, ${completed_time}, ${rewarded_time}, ${deleted}"
        ")";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBPlayerQuest::Load(AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "SELECT * FROM player_quests WHERE "
        "uuid = ${uuid} AND deleted = 0";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    g.uuid = result->GetString("uuid");
    g.playerUuid = result->GetString("player_uuid");
    g.questUuid = result->GetString("quests_uuid");
    g.completed = result->GetUInt("completed") != 0;
    g.rewarded = result->GetUInt("rewarded") != 0;
    g.progress = result->GetStream("progress");
    g.pickupTime = result->GetLong("picked_up_times");
    g.completeTime = result->GetLong("completed_time");
    g.rewardTime = result->GetLong("rewarded_time");
    g.deleted = result->GetUInt("deleted");

    return true;
}

bool DBPlayerQuest::Save(const AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    // Only these may be changed
    static constexpr const char* SQL = "UPDATE player_quests SET "
        "completed = ${completed}, "
        "rewarded = ${rewarded}, "
        "progress = ${progress}, "
        "picked_up_times = ${picked_up_times}, "
        "completed_time = ${completed_time}, "
        "rewarded_time = ${rewarded_time}, "
        "deleted = ${deleted} "
        "WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBPlayerQuest::Delete(const AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "DELETE FROM player_quests WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBPlayerQuest::Exists(const AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }
    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM player_quests WHERE "
        "uuid = ${uuid} AND deleted = 0";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
