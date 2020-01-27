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
#include "DBService.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBService::Create(AB::Entities::Service& s)
{
    if (Utils::Uuid::IsEmpty(s.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "INSERT INTO `services` (`uuid`, `name`, `type`, `location`, `host`, `port`, " <<
        "`status`, `start_time`, `stop_time`, `run_time`, " <<
        " `machine`, `file`, `path`, `arguments`) VALUES(";

    query << db->EscapeString(s.uuid) << ", ";
    query << db->EscapeString(s.name) << ", ";
    query << static_cast<int>(s.type) << ", ";
    query << db->EscapeString(s.location) << ", ";
    query << db->EscapeString(s.host) << ", ";
    query << s.port << ", ";
    query << static_cast<int>(s.status) << ", ";
    query << s.startTime << ", ";
    query << s.stopTime << ", ";
    query << s.runTime << ", ";
    query << db->EscapeString(s.machine) << ", ";
    query << db->EscapeString(s.file) << ", ";
    query << db->EscapeString(s.path) << ", ";
    query << db->EscapeString(s.arguments);

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

bool DBService::Load(AB::Entities::Service& s)
{
    if (Utils::Uuid::IsEmpty(s.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `services` WHERE ";
    query << "`uuid` = " << db->EscapeString(s.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    s.uuid = result->GetString("uuid");
    s.name = result->GetString("name");
    s.type = static_cast<AB::Entities::ServiceType>(result->GetUInt("type"));
    s.location = result->GetString("location");
    s.host = result->GetString("host");
    s.port = static_cast<uint16_t>(result->GetUInt("port"));
    s.status = static_cast<AB::Entities::ServiceStatus>(result->GetUInt("status"));
    s.startTime = result->GetLong("start_time");
    s.stopTime = result->GetLong("stop_time");
    s.runTime = result->GetLong("run_time");
    s.machine = result->GetString("machine");
    s.file = result->GetString("file");
    s.path = result->GetString("path");
    s.arguments = result->GetString("arguments");

    return true;
}

bool DBService::Save(const AB::Entities::Service& s)
{
    if (Utils::Uuid::IsEmpty(s.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `services` SET ";
    query << " `name` = " << db->EscapeString(s.name) << ", ";
    query << " `type` = " << static_cast<int>(s.type) << ", ";
    query << " `location` = " << db->EscapeString(s.location) << ", ";
    query << " `host` = " << db->EscapeString(s.host) << ", ";
    query << " `port` = " << s.port << ", ";
    query << " `status` = " << static_cast<int>(s.status) << ", ";
    query << " `start_time` = " << s.startTime << ", ";
    query << " `stop_time` = " << s.stopTime << ", ";
    query << " `run_time` = " << s.runTime << ", ";
    query << " `machine` = " << db->EscapeString(s.machine) << ", ";
    query << " `file` = " << db->EscapeString(s.file) << ", ";
    query << " `path` = " << db->EscapeString(s.path) << ", ";
    query << " `arguments` = " << db->EscapeString(s.arguments);

    query << " WHERE `uuid` = " << db->EscapeString(s.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBService::Delete(const AB::Entities::Service& s)
{
    if (Utils::Uuid::IsEmpty(s.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `services` WHERE `uuid` = " << db->EscapeString(s.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBService::Exists(const AB::Entities::Service& s)
{
    if (Utils::Uuid::IsEmpty(s.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `services` WHERE ";
    query << "`uuid` = " << db->EscapeString(s.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
