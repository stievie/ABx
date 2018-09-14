#pragma once

namespace Game {

class ScriptManager
{
private:
    static void LuaErrorHandler(int errCode, const char* message);
public:
    ScriptManager() = delete;

    static void RegisterLuaAll(kaguya::State& state);
    static bool FunctionExists(kaguya::State& state, const std::string& name)
    {
        return state[name].type() == LUA_TFUNCTION;
    }
};

}
