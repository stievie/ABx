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
class Actor;

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
    uint32_t index_;
    bool repeatable_;
    bool internalRewarded_{ false };
    bool internalDeleted_{ false };
    void InitializeLua();
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
    std::string _LuaGetVarString(const std::string& name);
    void _LuaSetVarString(const std::string& name, const std::string& value);
    float _LuaGetVarNumber(const std::string& name);
    void _LuaSetVarNumber(const std::string& name, float value);
    Player* _LuaGetOwner();
    void LoadProgress();
    /// A foe was killed nearby
    void OnKilledFoe(Actor* foe, Actor* killer);
public:
    static void RegisterLua(kaguya::State& state);

    Quest(Player& owner,
        const AB::Entities::Quest& q,
        AB::Entities::PlayerQuest&& playerQuest);
    // non-copyable
    Quest(const Quest&) = delete;
    Quest& operator=(const Quest&) = delete;

    ~Quest() = default;

    bool LoadScript(const std::string& fileName);

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);

    const Utils::Variant& GetVar(const std::string& name) const;
    void SetVar(const std::string& name, const Utils::Variant& val);

    bool IsCompleted() const { return playerQuest_.completed; }
    bool IsRewarded() const { return playerQuest_.rewarded; }
    bool IsRepeatable() const { return repeatable_; }
    void SaveProgress();
    bool CollectReward();
    bool Delete();
    bool IsActive() const;
    uint32_t GetIndex() const { return index_; }

    AB::Entities::PlayerQuest playerQuest_;
};

}
