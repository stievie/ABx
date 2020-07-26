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

namespace DB {

bool DBPlayerQuest::Create(AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO player_quests (uuid, quests_uuid, player_uuid, " <<
        "completed, rewarded, progress, picked_up_times, completed_time, rewarded_time, " <<
        "deleted";
    query << ") VALUES (";

    query << db->EscapeString(g.uuid) << ", ";
    query << db->EscapeString(g.questUuid) << ", ";
    query << db->EscapeString(g.playerUuid) << ", ";
    query << (g.completed ? 1 : 0) << ", ";
    query << (g.rewarded ? 1 : 0) << ", ";
    query << db->EscapeBlob(g.progress.data(), g.progress.length()) << ", ";
    query << g.pickupTime << ", ";
    query << g.completeTime << ", ";
    query << g.rewardTime << ", ";
    query << (g.deleted ? 1 : 0);

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

bool DBPlayerQuest::Load(AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    std::ostringstream query;
    Database* db = GetSubsystem<Database>();
    query << "SELECT * FROM player_quests WHERE ";
    query << "uuid = " << db->EscapeString(g.uuid) << " AND deleted = 0";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
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
    std::ostringstream query;

    query << "UPDATE player_quests SET ";

    // Only these may be changed
    query << " completed = " << (g.completed ? 1 : 0) << ", ";
    query << " rewarded = " << (g.rewarded ? 1 : 0) << ", ";
    query << " progress = " << db->EscapeBlob(g.progress.data(), g.progress.length()) << ", ";
    query << " picked_up_times = " << g.pickupTime << ", ";
    query << " completed_time = " << g.completeTime << ", ";
    query << " rewarded_time = " << g.rewardTime << ", ";
    query << " deleted = " << (g.deleted ? 1 : 0);

    query << " WHERE uuid = " << db->EscapeString(g.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
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
    std::ostringstream query;
    query << "DELETE FROM player_quests WHERE uuid = " << db->EscapeString(g.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBPlayerQuest::Exists(const AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid) || Utils::Uuid::IsEmpty(g.questUuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS count FROM player_quests WHERE ";
    query << "uuid = " << db->EscapeString(g.uuid) << " AND deleted = 0";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
