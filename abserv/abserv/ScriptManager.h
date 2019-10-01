#pragma once

#include <kaguya/kaguya.hpp>

namespace Game {
namespace Lua {

void RegisterLuaAll(kaguya::State& state);

/// Check if a function exists
inline bool IsFunction(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TFUNCTION;
}

inline bool IsVariable(kaguya::State& state, const std::string& name)
{
    auto t = state[name].type();
    return t == LUA_TBOOLEAN || t == LUA_TNUMBER || t == LUA_TSTRING;
}

inline bool IsString(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TSTRING;
}

inline bool IsBool(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TBOOLEAN;
}

inline bool IsNumber(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TNUMBER;
}

inline bool IsNil(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TNIL;
}

template<typename... _CArgs>
inline void CallFunction(kaguya::State& state, const std::string& name, _CArgs&& ... _Args)
{
    if (IsFunction(state, name))
        state[name](std::forward<_CArgs>(_Args)...);
}

void CollectGarbage(kaguya::State& state);

}
}
