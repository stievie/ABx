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

#include "DBInstance.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::GameInstance& inst, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(inst.uuid);
        if (token.value == "game_uuid")
            return db->EscapeString(inst.gameUuid);
        if (token.value == "server_uuid")
            return db->EscapeString(inst.serverUuid);
        if (token.value == "name")
            return db->EscapeString(inst.name);
        if (token.value == "recording")
            return db->EscapeString(inst.recording);
        if (token.value == "start_time")
            return std::to_string(inst.startTime);
        if (token.value == "stop_time")
            return std::to_string(inst.stopTime);
        if (token.value == "number")
            return std::to_string(inst.number);
        if (token.value == "is_running")
            return std::to_string(inst.running ? 1 : 0);
        if (token.value == "players")
            return std::to_string(inst.players);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBInstance::Create(AB::Entities::GameInstance& inst)
{
    if (Utils::Uuid::IsEmpty(inst.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "INSERT INTO instances ("
            "uuid, game_uuid, server_uuid, name, recording, start_time, stop_time, number, is_running, players"
        ") VALUES ("
            "${uuid}, ${game_uuid}, ${server_uuid}, ${name}, ${recording}, ${start_time}, ${stop_time}, ${number}, ${is_running}, ${players}"
        ")";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, inst, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBInstance::Load(AB::Entities::GameInstance& inst)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT * FROM instances WHERE ");
    if (!Utils::Uuid::IsEmpty(inst.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (!inst.recording.empty())
        parser.Append("recording = ${recording}", tokens);
    else
    {
        LOG_ERROR << "UUID and recording are empty" << std::endl;
        return false;
    }

    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, inst, std::placeholders::_1));
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    inst.uuid = result->GetString("uuid");
    inst.gameUuid = result->GetString("game_uuid");
    inst.serverUuid = result->GetString("server_uuid");
    inst.name = result->GetString("name");
    inst.recording = result->GetString("recording");
    inst.startTime = result->GetLong("start_time");
    inst.stopTime = result->GetLong("stop_time");
    inst.number = static_cast<uint16_t>(result->GetUInt("number"));
    inst.running = result->GetUInt("is_running") != 0;
    inst.players = static_cast<uint16_t>(result->GetUInt("players"));

    return true;
}

bool DBInstance::Save(const AB::Entities::GameInstance& inst)
{
    if (Utils::Uuid::IsEmpty(inst.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "UPDATE instances SET "
        "game_uuid = ${game_uuid}, "
        "server_uuid = ${server_uuid}, "
        "name = ${name}, "
        "recording = ${recording}, "
        "start_time = ${start_time}, "
        "stop_time = ${stop_time}, "
        "number = ${number}, "
        "is_running = ${is_running}, "
        "players = ${players} "
        "WHERE uuid = ${uuid}";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, inst, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBInstance::Delete(const AB::Entities::GameInstance& inst)
{
    if (Utils::Uuid::IsEmpty(inst.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "DELETE FROM instances WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, inst, std::placeholders::_1));
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBInstance::Exists(const AB::Entities::GameInstance& inst)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT COUNT(*) AS count FROM instances WHERE ");
    if (!Utils::Uuid::IsEmpty(inst.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (!inst.recording.empty())
        parser.Append("recording = ${recording}", tokens);
    else
    {
        LOG_ERROR << "UUID and recording are empty" << std::endl;
        return false;
    }

    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, inst, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

bool DBInstance::StopAll()
{
    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "UPDATE instances SET is_running = 0, stop_time = ${stop_time} WHERE is_running = 1";
    const std::string query = sa::templ::Parser::Evaluate(SQL, [](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
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
