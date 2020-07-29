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

#include "DBEffect.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Effect& effect, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(effect.uuid);
        if (token.value == "idx")
            return std::to_string(effect.index);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBEffect::Create(AB::Entities::Effect& effect)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(effect.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Load(AB::Entities::Effect& effect)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT * FROM game_effects WHERE ");
    if (!Utils::Uuid::IsEmpty(effect.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (effect.index != 0)
        parser.Append("idx = ${idx}", tokens);
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, effect, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    effect.uuid = result->GetString("uuid");
    effect.index = result->GetUInt("idx");
    effect.name = result->GetString("name");
    effect.category = static_cast<AB::Entities::EffectCategory>(result->GetUInt("category"));
    effect.script = result->GetString("script");
    effect.icon = result->GetString("icon");
    effect.soundEffect = result->GetString("sound_effect");
    effect.particleEffect = result->GetString("particle_effect");

    return true;
}

bool DBEffect::Save(const AB::Entities::Effect& effect)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(effect.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Delete(const AB::Entities::Effect& effect)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(effect.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Exists(const AB::Entities::Effect& effect)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT COUNT(*) AS count FROM game_effects WHERE ");
    if (!Utils::Uuid::IsEmpty(effect.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (effect.index != 0)
        parser.Append("idx = ${idx}", tokens);
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, effect, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
