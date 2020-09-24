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

#include "DBService.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Service& s, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(s.uuid);
        if (token.value == "name")
            return db->EscapeString(s.name);
        if (token.value == "type")
            return std::to_string(static_cast<int>(s.type));
        if (token.value == "location")
            return db->EscapeString(s.location);
        if (token.value == "host")
            return db->EscapeString(s.host);
        if (token.value == "port")
            return std::to_string(s.port);
        if (token.value == "status")
            return std::to_string(static_cast<int>(s.status));
        if (token.value == "start_time")
            return std::to_string(s.startTime);
        if (token.value == "stop_time")
            return std::to_string(s.stopTime);
        if (token.value == "run_time")
            return std::to_string(s.runTime);
        if (token.value == "machine")
            return db->EscapeString(s.machine);
        if (token.value == "file")
            return db->EscapeString(s.file);
        if (token.value == "path")
            return db->EscapeString(s.path);
        if (token.value == "arguments")
            return db->EscapeString(s.arguments);
        if (token.value == "version")
            return std::to_string(s.version);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBService::Create(AB::Entities::Service& s)
{
    if (Utils::Uuid::IsEmpty(s.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "INSERT INTO services ("
            "uuid, name, type, location, host, port, status, start_time, stop_time, run_time, machine, file, path, arguments, version"
        ") VALUES ("
            "${uuid}, ${name}, ${type}, ${location}, ${host}, ${port}, ${status}, ${start_time}, ${stop_time}, ${run_time}, ${machine}, ${file}, ${path}, ${arguments}, ${version}"
        ")";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, s, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBService::Load(AB::Entities::Service& s)
{
    if (Utils::Uuid::IsEmpty(s.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT * FROM services WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, s, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
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
    s.version = result->GetUInt("version");

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

    static constexpr const char* SQL = "UPDATE services SET "
        "name = ${name}, "
        "type = ${type}, "
        "location = ${location}, "
        "host = ${host}, "
        "port = ${port}, "
        "status = ${status}, "
        "start_time = ${start_time}, "
        "stop_time = ${stop_time}, "
        "run_time = ${run_time}, "
        "machine = ${machine}, "
        "file = ${file}, "
        "path = ${path}, "
        "arguments = ${arguments}, "
        "version = ${version} "
        "WHERE uuid = ${uuid}";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, s, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

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
    static constexpr const char* SQL = "DELETE FROM services WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, s, std::placeholders::_1));
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

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

    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM services WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, s, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

bool DBService::StopAll()
{
    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "UPDATE services SET status = ${status}, stop_time = ${stop_time} WHERE status = ${run_status}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, [](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "status")
                return std::to_string(static_cast<int>(AB::Entities::ServiceStatusOffline));
            if (token.value == "run_status")
                return std::to_string(static_cast<int>(AB::Entities::ServiceStatusOnline));
            if (token.value == "stop_time")
                return std::to_string(sa::time::tick());
            LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;

            return "";
        default:
            return token.value;
        }

    });
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

}
