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
#include "SimpleConfigManager.h"
#include "Logger.h"

namespace IO {

std::string SimpleConfigManager::GetGlobalString(const std::string& ident, const std::string& def)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isstring(L, -1))
    {
        lua_pop(L, 1);
        return def;
    }

    int len = (int)luaL_len(L, -1);
    std::string ret(lua_tostring(L, -1), len);
    lua_pop(L, 1);

    return ret;
}

int64_t SimpleConfigManager::GetGlobalInt(const std::string& ident, int64_t def)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isnumber(L, -1))
    {
        lua_pop(L, 1);
        return def;
    }

    int64_t val = static_cast<int64_t>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    return val;
}

float SimpleConfigManager::GetGlobalFloat(const std::string& ident, float def)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isnumber(L, -1))
    {
        lua_pop(L, 1);
        return def;
    }

    float val = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    return val;
}

bool SimpleConfigManager::GetGlobalBool(const std::string& ident, bool def)
{
    lua_getglobal(L, ident.c_str());

    if (!lua_isboolean(L, -1))
    {
        lua_pop(L, 1);
        return def;
    }

    bool val = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);

    return val;
}

bool SimpleConfigManager::Load(const std::string& file)
{
    if (L)
        lua_close(L);
    L = luaL_newstate();
    if (!L)
        return false;
    luaL_openlibs(L);

    if (luaL_dofile(L, file.c_str()) != 0)
    {
        int len = (int)luaL_len(L, -1);
        std::string err(lua_tostring(L, -1), len);
        LOG_ERROR << err << std::endl;
        lua_close(L);
        L = nullptr;
        return false;
    }

    isLoaded = true;
    return true;
}

}
