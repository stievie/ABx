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

#include "Party.h"
#include "Player.h"
#include "Actor.h"
#include "Chat.h"
#include "GameManager.h"
#include "PartyManager.h"
#include "Group.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <sa/EAIterator.h>
#include <sa/time.h>

namespace Game {

void Party::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Party"].setClass(kaguya::UserdataMetatable<Party, Group>()
        .addFunction("ChangeInstance", &Party::ChangeInstance)
        .addFunction("Defeat", &Party::Defeat)
        .addFunction("IsDefeated", &Party::IsDefeated)
        .addFunction("GetRandomPlayer", &Party::GetRandomPlayer)
        .addFunction("GetRandomPlayerInRange", &Party::GetRandomPlayerInRange)
        .addFunction("GetVarString", &Party::_LuaGetVarString)
        .addFunction("SetVarString", &Party::_LuaSetVarString)
        .addFunction("GetVarNumber", &Party::_LuaGetVarNumber)
        .addFunction("SetVarNumber", &Party::_LuaSetVarNumber)
    );
    // clang-format on
}

Party::Party() :
    Group(Group::GetNewId())
{
    chatChannel_ = ea::dynamic_pointer_cast<PartyChatChannel>(GetSubsystem<Chat>()->Get(ChatType::Party, id_));
    chatChannel_->party_ = this;
    members_.reserve(AB::Entities::Limits::MAX_PARTY_MEMBERS);
    // The Entity is created by the PartyManager
}

Party::~Party()
{
    GetSubsystem<Chat>()->Remove(ChatType::Party, id_);
}

size_t Party::GetDataPos(const Player& player)
{
   auto iter = ea::find_if(data_.members.begin(),
        data_.members.end(), [&player](const std::string& current)
    {
        return Utils::Uuid::IsEqual(player.data_.uuid, current);
    });
    const size_t index = std::distance(data_.members.begin(), iter);
    if (index == data_.members.size())
        return 0;
    // 1-based, 0 = invalid
    return index + 1;
}

std::string Party::_LuaGetVarString(const std::string& name)
{
    return GetVar(name).GetString();
}

void Party::_LuaSetVarString(const std::string& name, const std::string& value)
{
    SetVar(name, Utils::Variant(value));
}

float Party::_LuaGetVarNumber(const std::string& name)
{
    return GetVar(name).GetFloat();
}

void Party::_LuaSetVarNumber(const std::string& name, float value)
{
    SetVar(name, Utils::Variant(value));
}

bool Party::AddPlayer(ea::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsFull())
        return false;
    if (IsMember(*player))
        return false;

    members_.push_back(player);
    if (std::find(data_.members.begin(), data_.members.end(), player->data_.uuid) == data_.members.end())
        data_.members.push_back(player->data_.uuid);
    player->SetParty(shared_from_this());
    RemoveInvite(player);
    UpdateEntity(data_);
    return true;
}

bool Party::SetPlayer(ea::shared_ptr<Player> player)
{
    if (!player)
        return false;
    // 1-based
    const size_t pos = GetDataPos(*player);
    if (pos == 0)
    {
        // Not in data_ -> append it
        return AddPlayer(player);
    }
    if (pos == GetPosition(player.get()))
        // Already here
        return true;
    if (members_.size() < pos)
        members_.resize(pos);
    members_[pos - 1] = player;
    return true;
}

bool Party::RemovePlayer(Player& player, bool newParty /* = true */)
{
    members_.erase(ea::remove_if(members_.begin(), members_.end(), [&player](ea::weak_ptr<Actor>& current)
    {
        if (auto p = current.lock())
            return (p->id_ == player.id_);
        return false;
    }), members_.end());

    auto dataIt = ea::find(data_.members.begin(), data_.members.end(), player.data_.uuid);
    if (dataIt != data_.members.end())
        data_.members.erase(dataIt);
    UpdateEntity(data_);

    if (newParty)
    {
        // Lastly, this may call the destructor
        player.SetParty(ea::shared_ptr<Party>());
    }
    return true;
}

bool Party::Invite(ea::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsMember(*player) || IsInvited(*player))
        return false;

    invited_.push_back(player);
    return true;
}

bool Party::RemoveInvite(ea::shared_ptr<Player> player)
{
    if (!player)
        return false;

    auto it = ea::find_if(invited_.begin(), invited_.end(), [&player](const ea::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    if (it == invited_.end())
        return false;
    invited_.erase(it);
    return true;
}

void Party::ClearInvites()
{
    invited_.clear();
}

void Party::Update(uint32_t, Net::NetworkMessage& message)
{
    if (defeatedTick_ == 0)
    {
        size_t resigned = 0;
        VisitPlayers([&resigned] (Player& player) {
            if (player.IsResigned())
                ++resigned;
            return Iteration::Continue;
        });
        if (resigned == GetValidPlayerCount())
        {
            defeatedTick_ = sa::time::tick();
            message.AddByte(AB::GameProtocol::ServerPacketType::PartyResigned);
            AB::Packets::Server::PartyResigned packet = {
                id_,
                GetName()
            };
            AB::Packets::Add(packet, message);
            KillAll();
        }

        if (defeated_)
        {
            defeatedTick_ = sa::time::tick();
            message.AddByte(AB::GameProtocol::ServerPacketType::PartyDefeated);
            AB::Packets::Server::PartyDefeated packet = {
                id_,
                GetName()
            };
            AB::Packets::Add(packet, message);
            KillAll();
        }
    }

    if (defeatedTick_ != 0)
    {
        if (sa::time::time_elapsed(defeatedTick_) > PARTY_TELEPORT_BACK_TIME)
        {
            // Bring to the last outpost after 2 secs
            TeleportBack();
            defeatedTick_ = 0;
            defeated_ = false;
        }
    }
}

void Party::WriteToMembers(const Net::NetworkMessage& message)
{
    VisitPlayers([&message](Player& player) {
        player.WriteToOutput(message);
        return Iteration::Continue;
    });
}

void Party::SetPartySize(size_t size)
{
    while (members_.size() > size)
        members_.erase(members_.end());

    maxMembers_ = static_cast<uint32_t>(size);
}

inline size_t Party::GetValidPlayerCount() const
{
    size_t result = 0;
    VisitPlayers([&result](Player&) {
        ++result;
        return Iteration::Continue;
    });
    return result;
}

bool Party::IsMember(const Player& player) const
{
    auto it = ea::find_if(members_.begin(), members_.end(), [&player](const ea::weak_ptr<Actor>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player.id_;
        }
        return false;
    });
    return it != members_.end();
}

bool Party::IsInvited(const Player& player) const
{
    auto it = ea::find_if(invited_.begin(), invited_.end(), [&player](const ea::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player.id_;
        }
        return false;
    });
    return it != invited_.end();
}

bool Party::IsLeader(const Player& player) const
{
    if (members_.size() == 0)
        return false;
    if (auto p = members_[0].lock())
        return p->id_ == player.id_;
    return false;
}

Player* Party::GetLeader() const
{
    if (members_.size() == 0)
        return nullptr;
    if (auto p = members_[0].lock())
        // Note: Returns nullptr when its not a Player
        return To<Player>(p.get());
    return nullptr;
}

void Party::Defeat()
{
    defeated_ = true;
}

void Party::TeleportBack()
{
    auto member = GetAnyPlayer();
    if (member)
        ChangeInstance(member->data_.lastOutpostUuid);
}

size_t Party::GetPosition(const Actor* actor) const
{
    for (size_t i = 0; i < members_.size(); ++i)
    {
        if (auto sm = members_[i].lock())
        {
            if (sm->id_ == actor->id_)
                return (i + 1);
        }
    }
    return 0;
}

void Party::ChangeServerInstance(const std::string& serverUuid, const std::string& mapUuid, const std::string& instanceUuid)
{
    VisitPlayers([&](Player& player)
    {
        player.ChangeServerInstance(serverUuid, mapUuid, instanceUuid);
        return Iteration::Continue;
    });
}

void Party::NotifyPlayersQueued()
{
    ASSERT(GetLeader());
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::PlayerQueued),
        GetLeader()->GetName(),
        ""
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToMembers(*nmsg);
}

void Party::NotifyPlayersUnqueued()
{
    ASSERT(GetLeader());
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::PlayerUnqueued),
        GetLeader()->GetName(),
        ""
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToMembers(*nmsg);
}

const Utils::Variant& Party::GetVar(const std::string& name) const
{
    auto it = variables_.find(sa::StringHashRt(name.c_str()));
    if (it != variables_.end())
        return (*it).second;
    return Utils::Variant::Empty;
}

void Party::SetVar(const std::string& name, const Utils::Variant& val)
{
    variables_[sa::StringHashRt(name.c_str())] = val;
}

void Party::ChangeInstance(const std::string& mapUuid)
{
    // Get or create a game. The client gets an instance UUID to change to.
    ea::shared_ptr<Game> game = GetSubsystem<GameManager>()->GetGame(mapUuid, true);
    if (!game)
    {
        LOG_ERROR << "Failed to get game " << mapUuid << std::endl;
        return;
    }
    VisitPlayers([&mapUuid, &game] (Player& player) {
        player.ChangeInstance(mapUuid, game->instanceData_.uuid);
        return Iteration::Continue;
    });
}

ea::shared_ptr<Player> Party::GetAnyPlayer() const
{
    if (members_.size() == 0)
        return ea::shared_ptr<Player>();
    for (const auto& m : members_)
    {
        if (auto sm = m.lock())
        {
            if (Is<Player>(*sm))
                return To<Player>(*sm).GetPtr<Player>();
        }
    }
    return ea::shared_ptr<Player>();
}

Player* Party::GetRandomPlayer() const
{
    if (members_.size() == 0)
        return nullptr;

    ea::vector<Player*> players;
    VisitPlayers([&players](Player& current) {
       players.push_back(&current);
       return Iteration::Continue;
    });

    auto* rng = GetSubsystem<Crypto::Random>();
    const float rnd = rng->GetFloat();
    using iterator = ea::vector<Player*>::const_iterator;
    auto it = sa::ea::SelectRandomly<iterator>(players.begin(), players.end(), rnd);
    if (it != players.end())
        return (*it);
    return nullptr;
}

Player* Party::GetRandomPlayerInRange(const Actor* actor, Ranges range) const
{
    if (members_.size() == 0 || actor == nullptr)
        return nullptr;
    if (members_.size() == 1)
    {
        if (auto current = members_.at(0).lock())
        {
            if (Is<Player>(*current) && actor->IsInRange(range, current.get()))
                return To<Player>(current.get());
        }
        return nullptr;
    }

    ea::vector<Player*> players;
    VisitPlayers([&](Player& current) {
        if (actor->IsInRange(range, &current))
            players.push_back(&current);
        return Iteration::Continue;
    });
    if (players.size() == 0)
        return nullptr;

    auto* rng = GetSubsystem<Crypto::Random>();
    const float rnd = rng->GetFloat();
    using iterator = ea::vector<Player*>::const_iterator;
    auto it = sa::ea::SelectRandomly<iterator>(players.begin(), players.end(), rnd);
    if (it != players.end())
        return (*it);

    return nullptr;
}

void Party::VisitPlayers(const std::function<Iteration(Player& current)>& callback) const
{
    for (auto& m : members_)
    {
        if (auto sm = m.lock())
        {
            if (!Is<Player>(*sm))
                continue;
            auto& p = To<Player>(*sm);
            if (callback(p) != Iteration::Continue)
                break;
        }
    }
}

std::string Party::GetName() const
{
    auto* p = GetLeader();
    if (p)
        return p->GetName();
    return std::to_string(id_);
}

}
