#include "stdafx.h"
#include "Party.h"
#include "Player.h"

namespace Game {

Party::Party(std::shared_ptr<Player> leader) :
    leader_(leader),
    maxMembers_(8)
{
    leader->SetParty(shared_from_this());
}

bool Party::Add(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (members_.size() == maxMembers_)
        return false;
    if (IsMember(player))
        return false;

    members_.push_back(player);
    player->SetParty(shared_from_this());
    RemoveInvite(player);
    RemoveRequest(player);
    return true;
}

bool Party::Remove(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    auto it = std::find_if(members_.begin(), members_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    if (it == members_.end())
        return false;
    members_.erase(it);
    player->SetParty(std::shared_ptr<Party>());
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

bool Party::Request(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsMember(player) || IsRequestee(player))
        return false;

    requestees_.push_back(player);
    return true;
}

bool Party::RemoveRequest(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    auto it = std::find_if(requestees_.begin(), requestees_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    if (it == requestees_.end())
        return false;
    requestees_.erase(it);
    return true;
}

void Party::SetPartySize(uint32_t size)
{
    while (members_.size() > size)
    {
        auto it = members_.back();
        Remove(it.lock());
    }
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

bool Party::IsRequestee(std::shared_ptr<Player> player) const
{
    auto it = std::find_if(requestees_.begin(), requestees_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    return it != requestees_.end();
}

}
