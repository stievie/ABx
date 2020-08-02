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

#include "DBMusic.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Music& item, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(item.uuid);
        if (token.value == "map_uuid")
            return db->EscapeString(item.mapUuid);
        if (token.value == "local_file")
            return db->EscapeString(item.localFile);
        if (token.value == "remote_file")
            return db->EscapeString(item.remoteFile);
        if (token.value == "sorting")
            return std::to_string(item.sorting);
        if (token.value == "style")
            return std::to_string(item.style);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBMusic::Create(AB::Entities::Music& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "INSERT INTO game_music ("
            "uuid, map_uuid, local_file, remote_file, sorting, style"
        ") VALUES ("
            "${uuid}, ${map_uuid}, ${local_file}, ${remote_file}, ${sorting}, ${style}"
        ")";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBMusic::Load(AB::Entities::Music& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT * FROM game_music WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    item.uuid = result->GetString("uuid");
    item.mapUuid = result->GetString("map_uuid");
    item.localFile = result->GetString("local_file");
    item.remoteFile = result->GetString("remote_file");
    item.sorting = static_cast<uint8_t>(result->GetUInt("sorting"));
    item.style = static_cast<AB::Entities::MusicStyle>(result->GetUInt("style"));

    return true;
}

bool DBMusic::Save(const AB::Entities::Music& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "UPDATE game_music SET "
        "map_uuid = ${map_uuid}, "
        "local_file = ${local_file}, "
        "remote_file = ${remote_file}, "
        "sorting = ${sorting}, "
        "style = ${style} "
        "WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBMusic::Delete(const AB::Entities::Music& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "DELETE FROM game_music WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBMusic::Exists(const AB::Entities::Music& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM game_music WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
