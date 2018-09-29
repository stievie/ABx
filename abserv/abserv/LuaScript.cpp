#include "stdafx.h"
#include "LuaScript.h"
#include "Logger.h"
#include <lua.hpp>

namespace Game {

bool LuaScript::Execute(kaguya::State& luaState)
{
    vectorwrapbuf<char> databuf(buffer_);
    std::istream is(&databuf);
    if (!luaState.dostream(is))
    {
        LOG_ERROR << lua_tostring(luaState.state(), -1) << std::endl;
        return false;
    }
    return true;
}

}
