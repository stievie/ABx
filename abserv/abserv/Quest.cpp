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


#include "Quest.h"
#include "DataProvider.h"
#include "ItemFactory.h"
#include "Player.h"
#include "ProgressComp.h"
#include "Script.h"
#include "ScriptManager.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {

void Quest::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Quest"].setClass(kaguya::UserdataMetatable<Quest>()
        .addFunction("GetOwner", &Quest::_LuaGetOwner)
        .addFunction("IsCompleted", &Quest::IsCompleted)
        .addFunction("IsRewarded", &Quest::IsRewarded)
        .addFunction("GetVarString", &Quest::_LuaGetVarString)
        .addFunction("SetVarString", &Quest::_LuaSetVarString)
        .addFunction("GetVarNumber", &Quest::_LuaGetVarNumber)
        .addFunction("SetVarNumber", &Quest::_LuaSetVarNumber)
    );
    // clang-format on
}

Quest::Quest(Player& owner,
    const AB::Entities::Quest& q,
    AB::Entities::PlayerQuest&& playerQuest) :
    owner_(owner),
    index_(q.index),
    repeatable_(q.repeatable),
    playerQuest_(std::move(playerQuest))
{
    owner_.SubscribeEvent<void(Actor*, Actor*)>(EVENT_ON_KILLEDFOE, std::bind(&Quest::OnKilledFoe,
        this, std::placeholders::_1, std::placeholders::_2));
    InitializeLua();
    LoadProgress();
}

void Quest::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

bool Quest::LoadScript(const std::string& fileName)
{
    if (fileName.empty())
        return false;

    auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script)
        return false;
    if (!script->Execute(luaState_))
        return false;

    if (Lua::IsFunction(luaState_, "onUpdate"))
        sa::bits::set(functions_, FunctionUpdate);
    return true;
}

void Quest::LoadProgress()
{
    internalRewarded_ = playerQuest_.rewarded;
    internalDeleted_ = playerQuest_.deleted;
    sa::PropReadStream stream;
    stream.Init(playerQuest_.progress.data(), playerQuest_.progress.length());
    if (!Utils::VariantMapRead(variables_, stream))
    {
        LOG_WARNING << "Error loading Quest progress" << std::endl;
    }
}

void Quest::SaveProgress()
{
    sa::PropWriteStream stream;
    Utils::VariantMapWrite(variables_, stream);
    size_t ssize = 0;
    const char* s = stream.GetStream(ssize);
    playerQuest_.progress = std::string(s, ssize);
}

bool Quest::CollectReward()
{
    if (internalRewarded_ || playerQuest_.rewarded)
        return false;

    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Quest q;
    q.uuid = playerQuest_.questUuid;
    if (!client->Read(q))
        return false;
    owner_.progressComp_->AddXp(q.rewardXp);
    if (q.rewardMoney > 0)
    {
        if (!owner_.AddMoney(static_cast<uint32_t>(q.rewardMoney)))
            return false;
    }

    auto* factory = GetSubsystem<ItemFactory>();
    for (const auto& item : q.rewardItems)
    {
        uint32_t id = factory->CreatePlayerItem(owner_, item, 1);
        if (id != 0)
            owner_.AddToInventory(id);
    }

    internalRewarded_ = true;
    playerQuest_.rewardTime = Utils::Tick();
    return true;
}

void Quest::Update(uint32_t timeElapsed)
{
    if (playerQuest_.deleted)
        return;

    if (HaveFunction(FunctionUpdate))
        luaState_["onUpdate"](timeElapsed);
}

void Quest::Write(Net::NetworkMessage& message)
{
    if (playerQuest_.deleted)
        return;

    using namespace AB::GameProtocol;
    if (internalDeleted_ != playerQuest_.deleted)
    {
        playerQuest_.deleted = internalDeleted_;
        message.AddByte(ServerPacketType::QuestDeleted);
        AB::Packets::Server::QuestDeleted packet = {
            index_,
            playerQuest_.deleted
        };
        AB::Packets::Add(packet, message);
    }
    if (internalRewarded_ != playerQuest_.rewarded)
    {
        playerQuest_.rewarded = internalRewarded_;
        message.AddByte(ServerPacketType::QuestRewarded);
        AB::Packets::Server::QuestRewarded packet = {
            index_,
            playerQuest_.rewarded
        };
        AB::Packets::Add(packet, message);
    }
}

Player* Quest::_LuaGetOwner()
{
    return &owner_;
}

std::string Quest::_LuaGetVarString(const std::string& name)
{
    return GetVar(name).GetString();
}

void Quest::_LuaSetVarString(const std::string& name, const std::string& value)
{
    SetVar(name, Utils::Variant(value));
}

float Quest::_LuaGetVarNumber(const std::string& name)
{
    return GetVar(name).GetFloat();
}

void Quest::_LuaSetVarNumber(const std::string& name, float value)
{
    SetVar(name, Utils::Variant(value));
}

const Utils::Variant& Quest::GetVar(const std::string& name) const
{
    auto it = variables_.find(sa::StringHashRt(name.c_str()));
    if (it != variables_.end())
        return (*it).second;
    return Utils::Variant::Empty;
}

void Quest::SetVar(const std::string& name, const Utils::Variant& val)
{
    variables_[sa::StringHashRt(name.c_str())] = val;
}

void Quest::OnKilledFoe(Actor* foe, Actor* killer)
{
    Lua::CallFunction(luaState_, "onKilledFoe", foe, killer);
}

bool Quest::IsActive() const
{
    if (playerQuest_.deleted)
        return false;
    return !IsRewarded();
}

bool Quest::Delete()
{
    if (internalDeleted_ || playerQuest_.deleted ||
        internalRewarded_ || playerQuest_.rewarded)
        return false;
    internalDeleted_ = true;
    return true;
}

}
