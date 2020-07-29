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

#include "DBProfession.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallbackProfession(Database* db, const AB::Entities::Profession& prof, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid" || token.value == "profession_uuid")
            return db->EscapeString(prof.uuid);
        if (token.value == "idx")
            return std::to_string(prof.index);
        if (token.value == "name")
            return db->EscapeString(prof.name);
        if (token.value == "abbr")
            return db->EscapeString(prof.abbr);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBProfession::Create(AB::Entities::Profession& prof)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(prof.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBProfession::Load(AB::Entities::Profession& prof)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT * FROM game_professions WHERE ");
    if (!Utils::Uuid::IsEmpty(prof.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (prof.index != 0)
        parser.Append("idx = ${idx}", tokens);
    else if (!prof.name.empty())
        parser.Append("name = ${name}", tokens);
    else if (!prof.abbr.empty())
        parser.Append("abbr = ${abbr}", tokens);
    else
    {
        LOG_ERROR << "UUID, name, abbr and index are empty" << std::endl;
        return false;
    }
    const std::string query = tokens.ToString(std::bind(&PlaceholderCallbackProfession, db, prof, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    prof.uuid = result->GetString("uuid");
    prof.index = result->GetUInt("idx");
    prof.name = result->GetString("name");
    prof.abbr = result->GetString("abbr");
    prof.modelIndexFemale = result->GetUInt("model_index_female");
    prof.modelIndexMale = result->GetUInt("model_index_male");
    prof.position = static_cast<AB::Entities::ProfessionPosition>(result->GetUInt("position"));

    // Get attributes
    static constexpr const char* SQL_ATTR = "SELECT uuid, idx, is_primary FROM game_attributes WHERE profession_uuid = ${profession_uuid} ORDER BY idx";
    const std::string queryAttr = sa::templ::Parser::Evaluate(SQL_ATTR, std::bind(&PlaceholderCallbackProfession, db, prof, std::placeholders::_1));
    std::shared_ptr<DB::DBResult> resAttrib = db->StoreQuery(queryAttr);
    prof.attributeCount= 0;
    prof.attributes.clear();
    if (resAttrib)
    {
        for (; resAttrib; resAttrib = resAttrib->Next())
        {
            ++prof.attributeCount;
            prof.attributes.push_back({
                resAttrib->GetString("uuid"),
                resAttrib->GetUInt("idx"),
                resAttrib->GetInt("is_primary") != 0
            });
        }
    }
    return true;
}

bool DBProfession::Save(const AB::Entities::Profession& prof)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(prof.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBProfession::Delete(const AB::Entities::Profession& prof)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(prof.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBProfession::Exists(const AB::Entities::Profession& prof)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT COUNT(*) AS count FROM game_professions WHERE ");
    if (!Utils::Uuid::IsEmpty(prof.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (prof.index != 0)
        parser.Append("idx = ${idx}", tokens);
    else if (!prof.name.empty())
        parser.Append("name = ${name}", tokens);
    else if (!prof.abbr.empty())
        parser.Append("abbr = ${abbr}", tokens);
    else
    {
        LOG_ERROR << "UUID, name, abbr and index are empty" << std::endl;
        return false;
    }
    const std::string query = tokens.ToString(std::bind(&PlaceholderCallbackProfession, db, prof, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
