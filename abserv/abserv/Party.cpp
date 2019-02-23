#include "stdafx.h"
#include "Party.h"
#include "Player.h"
#include "Actor.h"
#include "Chat.h"
#include "GameManager.h"
#include "Subsystems.h"
#include "PartyManager.h"

namespace Game {

Utils::IdGenerator<uint32_t> Party::partyIds_;

Party::Party() :
    maxMembers_(1)
{
    id_ = GetNewId();
    chatChannel_ = std::dynamic_pointer_cast<PartyChatChannel>(GetSubsystem<Chat>()->Get(ChatType::Party, id_));
    chatChannel_->party_ = this;
    members_.reserve(AB::Entities::Limits::MAX_PARTY_MEMBERS);
    // The Entity is created by the PartyManager
}

Party::~Party()
{
    GetSubsystem<Chat>()->Remove(ChatType::Party, id_);
}

size_t Party::GetDataPos(Player* player)
{
    if (!player)
        return 0;
    std::vector<std::string>::iterator iter = std::find_if(data_.members.begin(),
        data_.members.end(), [player](const std::string& current)
    {
        return player->data_.uuid.compare(current) == 0;
    });
    size_t index = std::distance(data_.members.begin(), iter);
    if (index == data_.members.size())
    {
        return 0;
    }
    // 1-based, 0 = invalid
    return index + 1;
}

bool Party::Add(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsFull())
        return false;
    if (IsMember(player.get()))
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
    size_t pos = GetDataPos(player.get());
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

bool Party::Remove(Player* player, bool newParty /* = true */)
{
    if (!player)
        return false;

    members_.erase(std::remove_if(members_.begin(), members_.end(), [player](std::weak_ptr<Player>& current)
    {
        if (auto p = current.lock())
            return (p->id_ == player->id_);
        return false;
    }), members_.end());

    auto dataIt = std::find(data_.members.begin(), data_.members.end(), player->data_.uuid);
    if (dataIt != data_.members.end())
        data_.members.erase(dataIt);
    UpdateEntity(data_);

    if (newParty)
    {
        // Lastly, this may call the destructor
        player->SetParty(std::shared_ptr<Party>());
    }
    return true;
}

bool Party::Invite(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsMember(player.get()) || IsInvited(player.get()))
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

void Party::WriteToMembers(const Net::NetworkMessage& message)
{
    for (auto& wm : members_)
    {
        if (auto sm = wm.lock())
            sm->WriteToOutput(message);
    }
}

void Party::SetPartySize(size_t size)
{
    while (members_.size() > size)
        members_.erase(members_.end());

    maxMembers_ = static_cast<uint32_t>(size);
}

bool Party::IsMember(Player* player) const
{
    auto it = std::find_if(members_.begin(), members_.end(), [player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    return it != members_.end();
}

bool Party::IsInvited(Player* player) const
{
    auto it = std::find_if(invited_.begin(), invited_.end(), [player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    return it != invited_.end();
}

bool Party::IsLeader(Player* player) const
{
    if (members_.size() == 0)
        return false;
    if (auto p = members_[0].lock())
        return p->id_ == player->id_;
    return false;
}

size_t Party::GetPosition(Actor* actor)
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

void Party::ChangeInstance(const std::string& mapUuid)
{
    // Get or create a game. The client gets an instance UUID to change to.
    std::shared_ptr<Game> game = GetSubsystem<GameManager>()->GetGame(mapUuid, true);
    if (!game)
    {
        LOG_ERROR << "Failed to get game " << mapUuid << std::endl;
        return;
    }
    for (const auto& member : members_)
    {
        if (auto mem = member.lock())
        {
            mem->ChangeInstance(mapUuid, game->instanceData_.uuid);
        }
    }
}

}
