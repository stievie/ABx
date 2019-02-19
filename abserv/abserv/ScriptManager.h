#pragma once

namespace Game {

class ScriptManager
{
private:
    static void LuaErrorHandler(int errCode, const char* message);
public:
    ScriptManager() = delete;
    ~ScriptManager() = delete;

    static void RegisterLuaAll(kaguya::State& state);
    /// Check if a function exists
    static bool IsFunction(kaguya::State& state, const std::string& name)
    {
        return state[name].type() == LUA_TFUNCTION;
    }
    static bool IsVariable(kaguya::State& state, const std::string& name)
    {
        auto t = state[name].type();
        return t == LUA_TBOOLEAN || t == LUA_TNUMBER || t == LUA_TSTRING;
    }
    static bool IsString(kaguya::State& state, const std::string& name)
    {
        return state[name].type() == LUA_TSTRING;
    }
    static bool IsBool(kaguya::State& state, const std::string& name)
    {
        return state[name].type() == LUA_TBOOLEAN;
    }
    static bool IsNumber(kaguya::State& state, const std::string& name)
    {
        return state[name].type() == LUA_TNUMBER;
    }
    static bool IsNil(kaguya::State& state, const std::string& name)
    {
        return state[name].type() == LUA_TNIL;
    }
    template<typename... _CArgs>
    static void CallFunction(kaguya::State& state, const std::string& name, _CArgs&&... _Args)
    {
        if (IsFunction(state, name))
        {
            state[name](std::forward<_CArgs>(_Args)...);
        }
    }
};

}
