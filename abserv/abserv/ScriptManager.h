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
    static bool FunctionExists(kaguya::State& state, const std::string& name)
    {
        return state[name].type() == LUA_TFUNCTION;
    }
    static bool VariableExists(kaguya::State& state, const std::string& name)
    {
        auto t = state[name].type();
        return t == LUA_TBOOLEAN || t == LUA_TNUMBER || t == LUA_TSTRING;
    }
    static bool IsString(kaguya::State& state, const std::string& name)
    {
        return state[name].type() == LUA_TSTRING;
    }
};

}
