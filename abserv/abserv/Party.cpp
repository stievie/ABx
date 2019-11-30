#include "stdafx.h"
#include "Party.h"
#include "Player.h"
#include "Actor.h"
#include "Chat.h"
#include "GameManager.h"
#include "Subsystems.h"
#include "PartyManager.h"
#include "Random.h"
#include "UuidUtils.h"
#include "Group.h"

namespace Game {

void Party::RegisterLua(kaguya::State& state)
{
    state["Party"].setClass(kaguya::UserdataMetatable<Party>()
        .addFunction("GetLeader", &Party::_LuaGetLeader)
        .addFunction("ChangeInstance", &Party::ChangeInstance)
        .addFunction("Defeat", &Party::Defeat)
        .addFunction("IsDefeated", &Party::IsDefeated)
        .addFunction("KillAll", &Party::KillAll)
        .addFunction("GetRandomPlayer", &Party::GetRandomPlayer)
        .addFunction("GetRandomPlayerInRange", &Party::GetRandomPlayerInRange)
        .addFunction("GetMember", &Party::_LuaGetMember)
        .addFunction("GetMemberCount", &Party::_LuaGetMemberCount)
        .addFunction("GetVarString", &Party::_LuaGetVarString)
        .addFunction("SetVarString", &Party::_LuaSetVarString)
        .addFunction("GetVarNumber", &Party::_LuaGetVarNumber)
        .addFunction("SetVarNumber", &Party::_LuaSetVarNumber)
    );
}

Party::Party() :
    id_(Group::GetNewId())
{
    chatChannel_ = std::dynamic_pointer_cast<PartyChatChannel>(GetSubsystem<Chat>()->Get(ChatType::Party, id_));
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
    std::vector<std::string>::iterator iter = std::find_if(data_.members.begin(),
        data_.members.end(), [&player](const std::string& current)
    {
        return Utils::Uuid::IsEqual(player.data_.uuid, current);
    });
    const size_t index = std::distance(data_.members.begin(), iter);
    if (index == data_.members.size())
    {
        return 0;
    }
    // 1-based, 0 = invalid
    return index + 1;
}

Player* Party::_LuaGetLeader()
{
    auto leader = GetLeader();
    return leader ? leader.get() : nullptr;
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

Actor* Party::_LuaGetMember(int index)
{
    if (index >= static_cast<int>(members_.size()))
        return nullptr;
    if (auto m = members_.at(static_cast<size_t>(index)).lock())
        return m.get();
    return nullptr;
}

int Party::_LuaGetMemberCount()
{
    return static_cast<int>(members_.size());
}

bool Party::Add(std::shared_ptr<Player> player)
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

bool Party::Set(std::shared_ptr<Player> player)
{
    if (!player)
        return false;
    // 1-based
    const size_t pos = GetDataPos(*player);
    if (pos == 0)
    {
        // Not in data_ -> append it
        return Add(player);
    }
    if (pos == GetPosition(player.get()))
        // Already here
        return true;
    if (members_.size() < pos)
        members_.resize(pos);
    members_[pos - 1] = player;
    return true;
}

bool Party::Remove(Player& player, bool newParty /* = true */)
{
    members_.erase(std::remove_if(members_.begin(), members_.end(), [&player](std::weak_ptr<Player>& current)
    {
        if (auto p = current.lock())
            return (p->id_ == player.id_);
        return false;
    }), members_.end());

    auto dataIt = std::find(data_.members.begin(), data_.members.end(), player.data_.uuid);
    if (dataIt != data_.members.end())
        data_.members.erase(dataIt);
    UpdateEntity(data_);

    if (newParty)
    {
        // Lastly, this may call the destructor
        player.SetParty(std::shared_ptr<Party>());
    }
    return true;
}

bool Party::Invite(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsMember(*player) || IsInvited(*player))
        return false;

    invited_.push_back(player);
    return true;
}

bool Party::RemoveInvite(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    auto it = std::find_if(invited_.begin(), invited_.end(), [&player](const std::weak_ptr<Player>& current)
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
        VisitMembers([&resigned] (Player& player) {
            if (player.IsResigned())
                ++resigned;
            return Iteration::Continue;
        });
        if (resigned == GetValidMemberCount())
        {
            defeatedTick_ = Utils::Tick();
            message.AddByte(AB::GameProtocol::PartyResigned);
            message.Add<uint32_t>(id_);
            KillAll();
        }

        if (defeated_)
        {
            defeatedTick_ = Utils::Tick();
            message.AddByte(AB::GameProtocol::PartyDefeated);
            message.Add<uint32_t>(id_);
            KillAll();
        }
    }

    if (defeatedTick_ != 0)
    {
        if (Utils::TimeElapsed(defeatedTick_) > PARTY_TELEPORT_BACK_TIME)
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
    VisitMembers([&message](Player& player) {
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

inline size_t Party::GetValidMemberCount() const
{
    size_t result = 0;
    VisitMembers([&result](Player&) {
        ++result;
        return Iteration::Continue;
    });
    return result;
}

bool Party::IsMember(const Player& player) const
{
    auto it = std::find_if(members_.begin(), members_.end(), [&player](const std::weak_ptr<Player>& current)
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
    auto it = std::find_if(invited_.begin(), invited_.end(), [&player](const std::weak_ptr<Player>& current)
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

Player* Party::GetRandomPlayer() const
{
    if (members_.size() == 0)
        return nullptr;

    auto* rng = GetSubsystem<Crypto::Random>();
    const float rnd = rng->GetFloat();
    using iterator = std::vector<std::weak_ptr<Player>>::const_iterator;
    auto it = Utils::SelectRandomly<iterator>(members_.begin(), members_.end(), rnd);
    if (it != members_.end())
    {
        if (auto p = (*it).lock())
            return p.get();
    }
    return nullptr;
}

Player* Party::GetRandomPlayerInRange(const Actor* actor, Ranges range) const
{
    if (members_.size() == 0 || actor == nullptr)
        return nullptr;
    std::vector<Player*> players;
    VisitMembers([&](Player& player) {
        if (actor->IsInRange(range, &player))
            players.push_back(&player);
        return Iteration::Continue;
    });
    if (players.size() == 0)
        return nullptr;

    auto* rng = GetSubsystem<Crypto::Random>();
    const float rnd = rng->GetFloat();
    using iterator = std::vector<Player*>::const_iterator;
    auto it = Utils::SelectRandomly<iterator>(players.begin(), players.end(), rnd);
    if (it != players.end())
        return (*it);

    return nullptr;
}

void Party::KillAll()
{
    VisitMembers([](Player& player) {
        player.Die();
        return Iteration::Continue;
    });
}

void Party::Defeat()
{
    defeated_ = true;
}

void Party::TeleportBack()
{
    auto member = GetAnyMember();
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
    VisitMembers([&](Player& player)
    {
        player.ChangeServerInstance(serverUuid, mapUuid, instanceUuid);
        return Iteration::Continue;
    });
}

void Party::NotifyPlayersQueued()
{
    assert(GetLeader());
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypePlayerQueued);
    nmsg->AddString(GetLeader()->GetName());
    nmsg->AddString("");
    WriteToMembers(*nmsg);
}

void Party::NotifyPlayersUnqueued()
{
    assert(GetLeader());
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypePlayerUnqueued);
    nmsg->AddString(GetLeader()->GetName());
    nmsg->AddString("");
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
    std::shared_ptr<Game> game = GetSubsystem<GameManager>()->GetGame(mapUuid, true);
    if (!game)
    {
        LOG_ERROR << "Failed to get game " << mapUuid << std::endl;
        return;
    }
    VisitMembers([&mapUuid, &game] (Player& player) {
        player.ChangeInstance(mapUuid, game->instanceData_.uuid);
        return Iteration::Continue;
    });
}

}
