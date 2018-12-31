#include "stdafx.h"
#include "Party.h"
#include "Player.h"
#include "Actor.h"
#include "Chat.h"
#include "GameManager.h"
#include "Subsystems.h"

namespace Game {

Utils::IdGenerator<uint32_t> Party::partyIds_;

Party::Party(std::shared_ptr<Player> leader) :
    leader_(leader),
    maxMembers_(1)
{
    id_ = GetNewId();
    chatChannel_ = std::dynamic_pointer_cast<PartyChatChannel>(GetSubsystem<Chat>()->Get(ChatType::Party, id_));
    chatChannel_->party_ = this;
    members_.reserve(AB::Entities::Limits::MAX_PARTY_MEMBERS);
    members_.push_back(leader);
    data_.members.push_back(leader->data_.uuid);
}

Party::~Party()
{
    GetSubsystem<Chat>()->Remove(ChatType::Party, id_);
}

bool Party::Add(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsFull())
        return false;
    if (IsMember(player))
        return false;

    members_.push_back(player);
    player->SetParty(shared_from_this());
    RemoveInvite(player);
    return true;
}

bool Party::Remove(Player* player, bool newParty /* = true */)
{
    if (!player)
        return false;

    members_.erase(std::remove_if(members_.begin(), members_.end(), [&player](std::weak_ptr<Player>& current)
    {
        if (auto p = current.lock())
            return (p->id_ == player->id_);
        return true;
    }), members_.end());

    if (auto l = leader_.lock())
    {
        if (player->id_ == l->id_)
        {
            // Need new leader
            if (!members_.empty())
            {
                leader_ = members_.front().lock();
            }
        }
    }

    if (newParty)
        // Lastly, this may call the destructor
        player->SetParty(std::shared_ptr<Party>());
    return true;
}

bool Party::Invite(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsMember(player) || IsInvited(player))
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
            sm->client_->WriteToOutput(message);
    }
}

void Party::SetPartySize(size_t size)
{
    while (members_.size() > size)
        members_.erase(members_.end());

    maxMembers_ = static_cast<uint32_t>(size);
}

bool Party::IsMember(std::shared_ptr<Player> player) const
{
    auto it = std::find_if(members_.begin(), members_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    return it != members_.end();
}

bool Party::IsInvited(std::shared_ptr<Player> player) const
{
    auto it = std::find_if(invited_.begin(), invited_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    return it != invited_.end();
}

bool Party::IsLeader(Player* player)
{
    if (auto l = leader_.lock())
        return l.get() == player;
    return false;
}

uint8_t Party::GetPosition(const Actor* actor) const
{
    for (size_t i = 0; i < members_.size(); ++i)
    {
        if (auto sm = members_[i].lock())
        {
            if (sm->id_ == actor->id_)
                return (static_cast<uint8_t>(i + 1));
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
            mem->client_->ChangeInstance(mapUuid, game->instanceData_.uuid);
        }
    }
}

}
