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
#include "Item.h"
#include <string>
#include <base64.h>

Item::Item(Context* context) :
    Object(context)
{
}

Item::~Item()
{
}

void Item::LoadStatsFromString(const String& value)
{
    sa::PropReadStream stream;
    stream.Init(value.CString(), value.Length());
    if (!VariantMapRead(stats_, stream))
        URHO3D_LOGERROR("Error loading item stats");
}

String FormatMoney(uint32_t amount)
{
    auto src = std::to_string(amount);
    std::string result;

    auto count = 3;
    for (auto i = src.crbegin(); i != src.crend(); ++i)
    {
        if (count == 0)
        {
            result.push_back(',');
            count = 3;
        }
        if (count--)
            result.push_back(*i);
    }
    std::reverse(result.begin(), result.end());
    return String(result.c_str());
}

bool VariantMapRead(HashMap<Game::ItemStatIndex, Variant>& vMap, sa::PropReadStream& stream)
{
    vMap.Clear();
    if (stream.GetSize() == 0)
        // Empty but OK
        return true;

    uint16_t count = 0;
    if (!stream.Read<uint16_t>(count))
        return false;

    for (uint16_t i = 0; i < count; ++i)
    {
        uint64_t stat = 0;
        if (!stream.Read<uint64_t>(stat))
            return false;

        uint8_t bt = 0;
        if (!stream.Read<uint8_t>(bt))
            return false;
        VariantType t = static_cast<VariantType>(bt);

        if (t == VAR_NONE || t == VAR_VOIDPTR)
            continue;

        switch (t)
        {
        case VAR_INT:
        {
            int value = 0;
            if (stream.Read<int>(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = value;
            break;
        }
        case VAR_INT64:
        {
            long long value = 0;
            if (stream.Read<long long>(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = value;
            break;
        }
        case VAR_BOOL:
        {
            uint8_t value = 0;
            if (stream.Read<uint8_t>(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = value == 0 ? false : true;
            break;
        }
        case VAR_FLOAT:
        {
            float value = 0.0f;
            if (stream.Read<float>(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = value;
            break;
        }
        case VAR_STRING:
        {
            std::string value;
            if (stream.ReadString(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = String(value.c_str(), static_cast<unsigned>(value.length()));
            break;
        }
        default:
            break;
        }
    }
    return true;
}
