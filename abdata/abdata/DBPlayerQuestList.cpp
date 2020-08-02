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

#include "DBPlayerQuestList.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::PlayerQuestList& q, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "player_uuid")
            return db->EscapeString(q.uuid);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBPlayerQuestList::Create(AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestList::Load(AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT quests_uuid FROM player_quests WHERE "
        "player_uuid = ${player_uuid} AND rewarded = 0";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query); result; result = result->Next())
    {
        g.questUuids.push_back(
            result->GetString("quests_uuid")
        );
    }

    return true;
}

bool DBPlayerQuestList::Save(const AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestList::Delete(const AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestList::Exists(const AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM player_quests WHERE "
        "player_uuid = ${player_uuid} AND rewarded = 0";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
