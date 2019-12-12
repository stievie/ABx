#pragma once

#include "Variant.h"
#include <stdint.h>
#include <kaguya/kaguya.hpp>
#include <AB/Entities/Quest.h>
#include <AB/Entities/PlayerQuest.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Player;

class Quest
{
private:
    enum Function : uint32_t
    {
        FunctionNone,
        FunctionUpdate = 1,
    };
    uint32_t functions_{ FunctionNone };
    kaguya::State luaState_;
    Utils::VariantMap variables_;
    Player& owner_;
    void InitializeLua();
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
    std::string _LuaGetVarString(const std::string& name);
    void _LuaSetVarString(const std::string& name, const std::string& value);
    float _LuaGetVarNumber(const std::string& name);
    void _LuaSetVarNumber(const std::string& name, float value);
public:
    static void RegisterLua(kaguya::State& state);

    explicit Quest(Player& owner);
    // non-copyable
    Quest(const Quest&) = delete;
    Quest& operator=(const Quest&) = delete;

    ~Quest() = default;

    bool LoadScript(const std::string& fileName);

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);

    const Utils::Variant& GetVar(const std::string& name) const;
    void SetVar(const std::string& name, const Utils::Variant& val);

    AB::Entities::Quest data_;
    AB::Entities::PlayerQuest playerQuest_;
};

}
