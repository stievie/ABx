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


#include "IOScript.h"
#include <lua.hpp>
#include <llimits.h>
#include <sa/ScopeGuard.h>

namespace IO {

static int writer(lua_State*, const void* p, size_t size, void* u)
{
    if (size == 0)
        return 1;
    const char* addr = reinterpret_cast<const char*>(p);
    ea::vector<char>& buffer = reinterpret_cast<Game::Script*>(u)->GetBuffer();
    ea::copy(addr, addr + size, ea::back_inserter(buffer));
    return 0;
}

bool IOScript::Import(Game::Script& asset, const std::string& name)
{
    // https://stackoverflow.com/questions/8936369/compile-lua-code-store-bytecode-then-load-and-execute-it
    // https://stackoverflow.com/questions/17597816/lua-dump-in-c
    lua_State* L;
    L = luaL_newstate();

    asset.GetBuffer().clear();
    sa::ScopeGuard luaGuard([&L]()
    {
        lua_close(L);
    });

    int ret = luaL_loadfile(L, name.c_str());
    if (ret != LUA_OK)
    {
        LOG_ERROR << "Compile error: " << lua_tostring(L, -1) << std::endl;
        return false;
    }
    lua_lock(L);
    lua_dump(L, writer, &asset, 1);
    lua_unlock(L);

    return true;
}

}
