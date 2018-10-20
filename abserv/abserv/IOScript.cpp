#include "stdafx.h"
#include "IOScript.h"
#include <lua.hpp>
#include <llimits.h>

namespace IO {

static int writer(lua_State*, const void* p, size_t size, void* u)
{
    if (size == 0)
        return 1;
    const char* addr = reinterpret_cast<const char*>(p);
    std::vector<char>& buffer = reinterpret_cast<Game::Script*>(u)->GetBuffer();
    std::copy(addr, addr + size, std::back_inserter(buffer));
    return 0;
}

bool IOScript::Import(Game::Script* asset, const std::string& name)
{
    // https://stackoverflow.com/questions/8936369/compile-lua-code-store-bytecode-then-load-and-execute-it
    // https://stackoverflow.com/questions/17597816/lua-dump-in-c
    lua_State* L;
    L = luaL_newstate();
    int ret = luaL_loadfile(L, name.c_str());
    if (ret != LUA_OK)
    {
        LOG_ERROR << "Compile error: " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
        return false;
    }
    lua_lock(L);
    lua_dump(L, writer, asset, 1);
    lua_unlock(L);
    lua_close(L);

    return true;
}

}
