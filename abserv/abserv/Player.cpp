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

#include "stdafx.h"
#include "Application.h"
#include "Chat.h"
#include "FriendList.h"
#include "GameManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "IOAccount.h"
#include "IOGame.h"
#include "IOMail.h"
#include "IOPlayer.h"
#include "Item.h"
#include "ItemDrop.h"
#include "ItemFactory.h"
#include "ItemsCache.h"
#include "MailBox.h"
#include "Npc.h"
#include "Party.h"
#include "PartyManager.h"
#include "Player.h"
#include "PlayerManager.h"
#include "ProtocolGame.h"
#include "QuestComp.h"
#include "SkillManager.h"
#include <AB/Entities/AccountItemList.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/PlayerItemList.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <AB/ProtocolCodes.h>
#include <sa/StringTempl.h>

namespace Game {

void Player::RegisterLua(kaguya::State& state)
{
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Actor>()
        .addFunction("GetParty", &Player::_LuaGetParty)
        .addFunction("ChangeMap", &Player::ChangeMap)
        .addFunction("TriggerDialog", &Player::TriggerDialog)
        .addFunction("AddQuest", &Player::AddQuest)
        .addFunction("GetQuestReward", &Player::GetQuestReward)
        .addFunction("SatisfyQuestRequirements", &Player::SatisfyQuestRequirements)
    );
}

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    Actor(),
    client_(client),
    questComp_(std::make_unique<Components::QuestComp>(*this))
{
    events_.Subscribe<void(AB::GameProtocol::CommandType, const std::string&, Net::NetworkMessage&)>(EVENT_ON_HANDLECOMMAND,
        std::bind(&Player::OnHandleCommand, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    events_.Subscribe<void(void)>(EVENT_ON_INVENTORYFULL, std::bind(&Player::OnInventoryFull, this));
    events_.Subscribe<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
        std::bind(
            &Player::OnPingObject, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

Player::~Player() = default;

void Player::SetGame(std::shared_ptr<Game> game)
{
    Actor::SetGame(game);
    // Changing the instance also clears any invites. The client should check that we
    // leave the instance so don't send anything to invitees.
    party_->ClearInvites();
    if (game)
    {
        GetSubsystem<PartyManager>()->SetPartyGameId(party_->GetId(), game->id_);
        if (party_->IsLeader(*this))
            party_->SetPartySize(game->data_.partySize);
    }
}

size_t Player::GetGroupPos()
{
    return party_->GetPosition(this);
}

bool Player::CanAttack() const
{
    return !AB::Entities::IsOutpost(GetGame()->data_.type) ||
        account_.type >= AB::Entities::AccountTypeGamemaster;
}

bool Player::CanUseSkill() const
{
    return !AB::Entities::IsOutpost(GetGame()->data_.type) ||
        account_.type >= AB::Entities::AccountTypeGamemaster;
}

void Player::SetLevel(uint32_t value)
{
    data_.level = static_cast<uint8_t>(value);
    resourceComp_->UpdateResources();
}

void Player::AddXp(int value)
{
    assert(value >= 0);
    Actor::AddXp(value);
    data_.xp += static_cast<uint32_t>(value);
}

void Player::AddSkillPoint()
{
    Actor::AddSkillPoint();
    ++data_.skillPoints;
}

void Player::AdvanceLevel()
{
    if (data_.level < LEVEL_CAP)
        ++data_.level;
    Actor::AdvanceLevel();
}

void Player::Initialize()
{
    Actor::Initialize();
    SetParty(GetSubsystem<PartyManager>()->GetByUuid(data_.partyUuid));
    GetSubsystem<Asynch::Dispatcher>()->Add(Asynch::CreateTask(std::bind(&Player::LoadFriendList, this)));
}

void Player::Logout()
{
#ifdef DEBUG_GAME
    LOG_DEBUG << "Player logging out " << GetName() << std::endl;
#endif // DEBUG_GAME
    if (queueing_)
        CRQUnqueueForMatch();
    if (auto g = GetGame())
        g->PlayerLeave(id_);
    client_->Logout();
}

bool Player::SatisfyQuestRequirements(uint32_t index) const
{
    return questComp_->SatisfyRequirements(index);
}

bool Player::AddQuest(uint32_t index)
{
    return questComp_->PickupQuest(index);
}

bool Player::GetQuestReward(uint32_t index)
{
    return questComp_->GetReward(index);
}

void Player::CRQChangeMap(const std::string mapUuid)
{
    ChangeMap(mapUuid);
}

void Player::CRQLogout()
{
    Logout();
}

void Player::CRQPing(int64_t clientTick)
{
    lastPing_ = Utils::Tick();
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::GamePong);
    // Depending on the timezone of server and client the server may also be behind, i.e. difference is negative.
    AB::Packets::Server::Pong packet = { static_cast<int32_t>(clientTick - Utils::Tick()) };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::TriggerDialog(uint32_t dialogIndex)
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::DialogTrigger);
    AB::Packets::Server::DialogTrigger packet = {
        dialogIndex
    };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::TriggerQuestSelectionDialog(const std::set<uint32_t>& quests)
{
    if (quests.size() == 0)
        return;
    if (quests.size() == 1)
    {
        TriggerQuestDialog(*quests.begin());
        return;
    }
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::QuestSelectionDialogTrigger);
    AB::Packets::Server::QuestSelectionDialogTrigger packet;
    packet.count = static_cast<uint8_t>(quests.size());
    packet.quests.reserve(packet.count);
    for (auto i : quests)
        packet.quests.push_back(i);
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::TriggerQuestDialog(uint32_t index)
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::QuestDialogTrigger);
    AB::Packets::Server::QuestDialogTrigger packet = {
        index
    };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

MailBox& Player::GetMailBox()
{
    if (!mailBox_)
        mailBox_ = std::make_unique<MailBox>(data_.accountUuid);
    return *mailBox_;
}

void Player::UpdateMailBox()
{
    GetMailBox().Update();
}

void Player::CRQGetMailHeaders()
{
    UpdateMailBox();

    const MailBox& mailBox = GetMailBox();
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::MailHeaders);
    AB::Packets::Server::MailHeaders packet;
    packet.count = static_cast<uint16_t>(mailBox.GetTotalMailCount());
    packet.headers.reserve(packet.count);
    const AB::Entities::MailList& mails = mailBox.GetMails();
    for (const auto& mail : mails.mails)
    {
        packet.headers.push_back({
            mail.uuid, mail.fromName, mail.subject, mail.created, mail.isRead
        });
    }
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::CRQGetInventory()
{
    const size_t count = inventoryComp_->GetInventoryCount();
    if (count == 0)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::InventoryContent);
    AB::Packets::Server::InventoryContent packet;
    packet.count = static_cast<uint16_t>(count);

    inventoryComp_->VisitInventory([&packet](const Item& current)
    {
        packet.items.push_back({
            static_cast<uint16_t>(current.data_.type),
            current.data_.index,
            static_cast<uint8_t>(current.concreteItem_.storagePlace),
            current.concreteItem_.storagePos,
            current.concreteItem_.count,
            current.concreteItem_.value
        });
        return Iteration::Continue;
    });
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::CRQDestroyInventoryItem(uint16_t pos)
{
    if (!inventoryComp_->DestroyInventoryItem(pos))
        return;

    AB::Entities::InventoryItems inv;
    inv.uuid = data_.uuid;
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    cli->Invalidate(inv);

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
    AB::Packets::Server::InventoryItemDelete packet = {
        pos
    };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::EquipInventoryItem(uint16_t pos)
{
    EquipPos ePos = inventoryComp_->EquipInventoryItem(pos);
    if (ePos == EquipPos::None)
        return;

    IO::DataClient* cli = GetSubsystem<IO::DataClient>();

    AB::Entities::InventoryItems inv;
    inv.uuid = data_.uuid;
    cli->Invalidate(inv);

    auto msg = Net::NetworkMessage::GetNew();
    // Removed from inventory
    msg->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
    AB::Packets::Server::InventoryItemDelete packet = {
        pos
    };
    AB::Packets::Add(packet, *msg);
    // TODO: Write new equipment
//    msg->AddByte(AB::GameProtocol::ServerPacketType::EquipmentItemUpdate);
//    msg->Add<uint16_t>(static_cast<uint16_t>(ePos));

    WriteToOutput(*msg);

    AB::Entities::EquippedItems equ;
    equ.uuid = data_.uuid;
    cli->Invalidate(equ);
}

void Player::CRQStoreInChest(uint16_t pos)
{
    if (inventoryComp_->IsChestFull())
    {
        OnInventoryFull();
        return;
    }

    uint32_t itemId = inventoryComp_->RemoveInventoryItem(pos);
    if (itemId == 0)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    // Remove from inventory
    msg->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
    AB::Packets::Server::InventoryItemDelete packet = {
        pos
    };
    AB::Packets::Add(packet, *msg);

    // Add to chest
    inventoryComp_->SetChestItem(itemId, msg.get());
    WriteToOutput(*msg);
}

void Player::CRQDropInventoryItem(uint16_t pos)
{
    uint32_t itemId = inventoryComp_->RemoveInventoryItem(pos);
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (!item)
        return;

    item->concreteItem_.storagePlace = AB::Entities::StoragePlaceScene;
    item->concreteItem_.storagePos = 0;
    auto rng = GetSubsystem<Crypto::Random>();
    std::shared_ptr<ItemDrop> drop = std::make_shared<ItemDrop>(item->id_);
    drop->transformation_.position_ = transformation_.position_;
    // Random pos around dropper
    drop->transformation_.position_.y_ += 0.2f;
    drop->transformation_.position_.x_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
    drop->transformation_.position_.z_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
    drop->SetSource(GetPtr<Actor>());
    GetGame()->SpawnItemDrop(drop);

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
    AB::Packets::Server::InventoryItemDelete packet = {
        pos
    };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::CRQGetChest()
{
    const size_t count = inventoryComp_->GetChestCount();
    if (count == 0)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::ChestContent);
    AB::Packets::Server::InventoryContent packet;
    packet.count = static_cast<uint16_t>(count);
    packet.items.reserve(packet.count);
    inventoryComp_->VisitChest([&packet](const Item& current)
    {
        packet.items.push_back({
            current.data_.type,
            current.data_.index,
            static_cast<uint8_t>(current.concreteItem_.storagePlace),
            current.concreteItem_.storagePos,
            current.concreteItem_.count,
            current.concreteItem_.value
        });
        return Iteration::Continue;
    });
    AB::Packets::Add(packet, *msg);

    WriteToOutput(*msg);
}

void Player::CRQDestroyChestItem(uint16_t pos)
{
    if (inventoryComp_->DestroyChestItem(pos))
    {
        AB::Entities::ChestItems inv;
        inv.uuid = account_.uuid;
        IO::DataClient* cli = GetSubsystem<IO::DataClient>();
        cli->Invalidate(inv);

        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ChestItemDelete);
        AB::Packets::Server::InventoryItemDelete packet = {
            pos
        };
        AB::Packets::Add(packet, *msg);
        WriteToOutput(*msg);
    }
}

void Player::CRQSendMail(const std::string recipient, const std::string subject, const std::string body)
{
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet;
    // Can not send mails to players I ignore
    if (!IsIgnored(recipient) && IO::IOMail::SendMailToPlayer(recipient, data_.accountUuid, GetName(), subject, body))
        packet.type = static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::MailSent);
    else
        packet.type = static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::MailNotSent);
    packet.sender = recipient;
    AB::Packets::Add(packet, *nmsg);

    WriteToOutput(*nmsg);
}

void Player::CRQGetMail(const std::string mailUuid)
{
    UpdateMailBox();

    // mailUuid must not be a reference!
    AB::Entities::Mail m;
    if (GetMailBox().ReadMail(mailUuid, m))
    {
        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::MailComplete);
        AB::Packets::Server::MailComplete packet = {
            m.fromAccountUuid,
            m.fromName,
            m.toName,
            m.subject,
            m.message,
            m.created,
            m.isRead
        };
        AB::Packets::Add(packet, *msg);
        WriteToOutput(*msg);
    }
}

void Player::CRQDeleteMail(const std::string mailUuid)
{
    // mailUuid must not be a reference!
    UpdateMailBox();

    if (mailUuid.compare("all") == 0)
    {
        auto msg = Net::NetworkMessage::GetNew();
        GetMailBox().DeleteAll();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
        AB::Packets::Server::ServerMessage packet = {
            static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::MailDeleted),
            GetName(),
            mailUuid
        };
        AB::Packets::Add(packet, *msg);
        WriteToOutput(*msg);
        return;
    }

    AB::Entities::Mail m;
    if (GetMailBox().DeleteMail(mailUuid, m))
    {
        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
        AB::Packets::Server::ServerMessage packet = {
            static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::MailDeleted),
            GetName(),
            mailUuid
        };
        AB::Packets::Add(packet, *msg);
        WriteToOutput(*msg);
    }
}

void Player::NotifyNewMail()
{
    UpdateMailBox();

    const MailBox& mailBox = GetMailBox();
    auto msg = Net::NetworkMessage::GetNew();
    if (mailBox.GetTotalMailCount() > 0)
    {
        // Notify player there are new emails since last check.
        msg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
        AB::Packets::Server::ServerMessage packet = {
            static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::NewMail),
            GetName(),
            std::to_string(mailBox.GetTotalMailCount())
        };
        AB::Packets::Add(packet, *msg);
    }
    if (mailBox.GetTotalMailCount() >= AB::Entities::Limits::MAX_MAIL_COUNT)
    {
        // Notify player that mailbox is full.
        msg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
        AB::Packets::Server::ServerMessage packet = {
            static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::MailboxFull),
            GetName(),
            std::to_string(mailBox.GetTotalMailCount())
        };
        AB::Packets::Add(packet, *msg);
    }
    if (msg->GetSize() != 0)
        WriteToOutput(*msg);
}

void Player::CRQAddFriend(const std::string playerName, AB::Entities::FriendRelation relation)
{
    auto res = GetFriendList().AddFriendByName(playerName, relation);

    auto msg = Net::NetworkMessage::GetNew();
    switch (res)
    {
    case FriendList::Error::Success:
    {
        AB::Entities::Friend f;
        GetFriendList().GetFriendByName(playerName, f);
        msg->AddByte(AB::GameProtocol::ServerPacketType::FriendAdded);
        AB::Packets::Server::FriendAdded packet = {
            f.friendUuid,
            static_cast<AB::Packets::Server::PlayerInfo::Relation>(f.relation)
        };
        AB::Packets::Add(packet, *msg);
        break;
    }
    case FriendList::Error::NoFriend:
        // N/A
    case FriendList::Error::AlreadyFriend:
    case FriendList::Error::InternalError:
        // Do nothing
        break;
    case FriendList::Error::PlayerNotFound:
    {
        msg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
        AB::Packets::Server::ServerMessage packet = {
            static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::PlayerNotFound),
            GetName(),
            playerName
        };
        AB::Packets::Add(packet, *msg);
        break;
    }
    }

    WriteToOutput(*msg);
}

void Player::CRQRemoveFriend(const std::string accountUuid)
{
    AB::Entities::Friend f;

    if (!GetFriendList().GetFriendByAccount(accountUuid, f))
        return;

    auto res = GetFriendList().Remove(accountUuid);

    auto msg = Net::NetworkMessage::GetNew();
    switch (res)
    {
    case FriendList::Error::Success:
    {
        msg->AddByte(AB::GameProtocol::ServerPacketType::FriendRemoved);
        AB::Packets::Server::FriendRemoved packet = {
            accountUuid,
            static_cast<AB::Packets::Server::PlayerInfo::Relation>(f.relation)
        };
        AB::Packets::Add(packet, *msg);
        break;
    }
    case FriendList::Error::AlreadyFriend:
        // N/A
    case FriendList::Error::NoFriend:
    case FriendList::Error::InternalError:
    case FriendList::Error::PlayerNotFound:
        // Do nothing
        break;
    }

    WriteToOutput(*msg);
}

void Player::CRQChangeFriendNick(const std::string accountUuid, const std::string newName)
{
    AB::Entities::Friend f;

    if (!GetFriendList().GetFriendByAccount(accountUuid, f))
        return;

    auto res = GetFriendList().ChangeNickname(accountUuid, newName);

    auto msg = Net::NetworkMessage::GetNew();
    switch (res)
    {
    case FriendList::Error::Success:
    {
        msg->AddByte(AB::GameProtocol::ServerPacketType::FriendRenamed);
        AB::Packets::Server::FriendRenamed packet = {
            accountUuid,
            static_cast<AB::Packets::Server::PlayerInfo::Relation>(f.relation),
            newName
        };
        AB::Packets::Add(packet, *msg);
        break;
    }
    case FriendList::Error::PlayerNotFound:
    {
        // Do nothing
        msg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
        AB::Packets::Server::ServerMessage packet = {
            static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::PlayerNotFound),
            GetName(),
            accountUuid
        };
        AB::Packets::Add(packet, *msg);
        break;
    }
    case FriendList::Error::AlreadyFriend:
        // N/A
    case FriendList::Error::NoFriend:
    case FriendList::Error::InternalError:
        // Do nothing
        break;
    }

    WriteToOutput(*msg);
}

void Player::SendPlayerInfo(const AB::Entities::Character& ch, uint32_t fields)
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::PlayerInfo);
    AB::Packets::Server::PlayerInfo packet;
    packet.fields = fields;
    packet.accountUuid = ch.accountUuid;

    AB::Entities::Friend f;
    bool isFriend = GetFriendList().GetFriendByAccount(ch.accountUuid, f);
    if (isFriend)
        packet.nickName = f.friendName;
    else
        packet.nickName = ch.name;
    packet.relation = static_cast<AB::Packets::Server::PlayerInfo::Relation>(f.relation);

    AB::Entities::Account account;
    account.uuid = ch.accountUuid;
    AB::Entities::Character currentToon;
    IO::IOAccount::GetAccountInfo(account, currentToon);
    if (f.relation != AB::Entities::FriendRelationIgnore && account.onlineStatus != AB::Entities::OnlineStatusInvisible)
    {
        // If success == false -> offline, empty toon name
        packet.status = static_cast<AB::Packets::Server::PlayerInfo::Status>(account.onlineStatus);
        packet.currentName = currentToon.name;
        if (IO::IOPlayer::HasFriendedMe(account_.uuid, account.uuid))
            // Send only location when I'm in their FL
            packet.currentMap = currentToon.currentMapUuid;
        else
            packet.currentMap = Utils::Uuid::EMPTY_UUID;
    }
    else
    {
        // Ignored always offline and no current toon
        packet.status = AB::Packets::Server::PlayerInfo::OnlineStatusOffline;
        packet.currentMap = Utils::Uuid::EMPTY_UUID;
    }
    // Guild info
    AB::Entities::GuildMember gm;
    IO::IOAccount::GetGuildMemberInfo(account, gm);
    packet.guildUuid = account.guildUuid;
    packet.guildRole = static_cast<AB::Packets::Server::PlayerInfo::GuildRole>(gm.role);
    packet.guildInviteName = gm.inviteName;
    packet.invited = gm.invited;
    packet.joined = gm.joined;
    packet.expires = gm.expires;

    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::CRQGetPlayerInfoByAccount(const std::string accountUuid, uint32_t fields)
{
    AB::Entities::Character ch;
    bool found = IO::IOPlayer::GetPlayerInfoByAccount(accountUuid, ch);
    if (!found)
        // If there is no such thing, we just don't reply to this request
        return;
    SendPlayerInfo(ch, fields);
}

void Player::CRQGetPlayerInfoByName(const std::string name, uint32_t fields)
{
    AB::Entities::Character ch;
    bool found = IO::IOPlayer::GetPlayerInfoByName(name, ch);
    if (!found)
        // If there is no such thing, we just don't reply to this request
        return;
    SendPlayerInfo(ch, fields);
}

void Player::CRQGetGuildMembers()
{
    auto* gm = GetSubsystem<GuildManager>();
    auto guild = gm->Get(account_.guildUuid);
    if (!guild)
        return;

    AB::Entities::GuildMembers members;
    if (!guild->GetMembers(members))
        return;
    if (members.members.size() == 0)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::GuildMemberList);
    AB::Packets::Server::GuildMemberList packet;
    packet.count = static_cast<uint16_t>(members.members.size());
    packet.members.reserve(packet.count);

    for (const AB::Entities::GuildMember& member : members.members)
        packet.members.push_back(member.accountUuid);
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::CRQGetGuildInfo()
{
    auto* gm = GetSubsystem<GuildManager>();
    auto guild = gm->Get(account_.guildUuid);
    if (!guild)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::GuildInfo);
    AB::Packets::Server::GuildInfo packet = {
        guild->data_.uuid,
        guild->data_.name,
        guild->data_.tag,
        guild->data_.creation,
        guild->data_.creatorAccountUuid
    };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::CRQGetFriendList()
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::FriendList);
    AB::Packets::Server::FriendList packet;
    packet.count = static_cast<uint16_t>(GetFriendList().Count());
    packet.friends.reserve(packet.count);
    GetFriendList().VisitAll([&packet](const AB::Entities::Friend& current)
    {
        packet.friends.push_back(current.friendUuid);
        return Iteration::Continue;
    });
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::WriteToOutput(const Net::NetworkMessage& message)
{
    if (!client_)
    {
        LOG_ERROR << "client_ expired" << std::endl;
        return;
    }

    if (message.GetSize() != 0)
        client_->WriteToOutput(message);
}

void Player::OnPingObject(uint32_t targetId, AB::GameProtocol::ObjectCallType type, int skillIndex)
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::GameObjectPingTarget);
    AB::Packets::Server::ObjectPingTarget packet = {
        id_,
        targetId,
        static_cast<uint8_t>(type),
        static_cast<int8_t>(skillIndex)
    };
    AB::Packets::Add(packet, *msg);
    GetParty()->WriteToMembers(*msg);
}

void Player::OnInventoryFull()
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::PlayerError);
    AB::Packets::Server::GameError packet = {
        static_cast<uint8_t>(AB::GameProtocol::PlayerErrorInventoryFull)
    };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::SetParty(std::shared_ptr<Party> party)
{
    if (party_)
    {
        if (party && (party_->GetId() == party->GetId()))
            return;
        party_->RemovePlayer(*this, false);
        SetGroupId(0);
    }

    if (party)
    {
        party_ = party;
        data_.partyUuid = party->data_.uuid;
        SetGroupId(party_->GetId());
    }
    else
    {
        // Create new party
        data_.partyUuid.clear();
        party_ = GetSubsystem<PartyManager>()->GetByUuid(data_.partyUuid);
        party_->SetPartySize(GetGame()->data_.partySize);
        data_.partyUuid = party_->data_.uuid;
        SetGroupId(party_->GetId());
    }
    party_->SetPlayer(GetPtr<Player>());
}

void Player::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    Actor::Update(timeElapsed, message);
    questComp_->Update(timeElapsed);
    questComp_->Write(message);
    auto party = GetParty();
    if (party->IsLeader(*this))
        party->Update(timeElapsed, message);
}

bool Player::RemoveMoney(uint32_t count)
{
    (void)count;
    return true;
}

bool Player::AddMoney(uint32_t count)
{
    auto* factory = GetSubsystem<ItemFactory>();
    uint32_t id = factory->CreatePlayerMoneyItem(*this, count);
    if (id == 0)
        return false;
    return AddToInventory(id);
}

bool Player::AddToInventory(uint32_t itemId)
{
    if (inventoryComp_->IsInventoryFull())
    {
        OnInventoryFull();
        return false;
    }
    auto msg = Net::NetworkMessage::GetNew();
    const bool ret = inventoryComp_->SetInventoryItem(itemId, msg.get());
    if (msg->GetSize() != 0)
        WriteToOutput(*msg);
    return ret;
}

void Player::CRQPartyInvitePlayer(uint32_t playerId)
{
    // The leader invited a player
    if (!AB::Entities::IsOutpost(GetGame()->data_.type))
        return;
    if (id_ == playerId)
        return;
    if (!party_->IsLeader(*this))
        return;
    if (party_->IsFull())
        return;
    std::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (player)
    {
        if (party_->Invite(player))
        {
            auto nmsg = Net::NetworkMessage::GetNew();
            nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyPlayerInvited);
            AB::Packets::Server::PartyPlayerInvited packet = {
                id_,
                playerId,
                party_->GetId()
            };
            AB::Packets::Add(packet, *nmsg);
            // Send us confirmation
            party_->WriteToMembers(*nmsg);
            // Send player he was invited
            player->WriteToOutput(*nmsg);
        }
    }
}

void Player::CRQPartyKickPlayer(uint32_t playerId)
{
    // The leader kicks a player from the party
    if (!AB::Entities::IsOutpost(GetGame()->data_.type))
        return;
    if (id_ == playerId)
        // Can not kick myself
        return;
    if (!party_->IsLeader(*this))
        // Only leader can kick
        return;

    std::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (!player)
        return;

    bool removedMember = false;
    {
        auto nmsg = Net::NetworkMessage::GetNew();
        if (party_->IsMember(*player))
        {
            if (!party_->RemovePlayer(*player))
                return;
            nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyPlayerRemoved);
            removedMember = true;
        }
        else if (party_->IsInvited(*player))
        {
            if (!party_->RemoveInvite(player))
                return;
            nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyInviteRemoved);
        }
        else
            return;

        AB::Packets::Server::PartyPlayerRemoved packet = {
            id_,
            playerId,
            party_->GetId()
        };
        AB::Packets::Add(packet, *nmsg);
        party_->WriteToMembers(*nmsg);

        // Also send to player which is removed already
        player->WriteToOutput(*nmsg);
    }

    if (removedMember)
    {
        // The kicked player needs a new party
        player->SetParty(std::shared_ptr<Party>());
        auto nmsg = Net::NetworkMessage::GetNew();
        nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyPlayerAdded);
        AB::Packets::Server::PartyPlayerAdded packet = {
            player->id_,
            player->id_,
            player->GetParty()->GetId()
        };
        AB::Packets::Add(packet, *nmsg);
        player->GetParty()->WriteToMembers(*nmsg);
    }
}

void Player::PartyLeave()
{
    if (party_->IsLeader(*this) && party_->GetMemberCount() == 1)
        // Just we
        return;

    {
        auto nmsg = Net::NetworkMessage::GetNew();
        nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyPlayerRemoved);
        auto leader = party_->GetLeader();
        AB::Packets::Server::PartyPlayerRemoved packet = {
            (leader ? leader->id_ : 0),
            id_,
            party_->GetId()
        };
        AB::Packets::Add(packet, *nmsg);
        party_->WriteToMembers(*nmsg);
        party_->RemovePlayer(*this);
    }

    {
        // We need a new party
        SetParty(std::shared_ptr<Party>());
        auto nmsg = Net::NetworkMessage::GetNew();
        nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyPlayerAdded);
        AB::Packets::Server::PartyPlayerAdded packet = {
            id_,
            id_,
            party_->GetId()
        };
        AB::Packets::Add(packet, *nmsg);
        party_->WriteToMembers(*nmsg);
    }
}

void Player::CRQPartyLeave()
{
    PartyLeave();
}

void Player::CRQPartyAccept(uint32_t playerId)
{
    // Sent by the acceptor to the leader of the party that a player accepted
    if (!AB::Entities::IsOutpost(GetGame()->data_.type))
        return;

    std::shared_ptr<Player> leader = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (leader)
    {
        // Leave current party
        PartyLeave();
        if (leader->GetParty()->Add(GetPtr<Player>()))
        {
            auto nmsg = Net::NetworkMessage::GetNew();
            nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyPlayerAdded);
            AB::Packets::Server::PartyPlayerAdded packet = {
                id_,
                playerId,
                party_->GetId()
            };
            AB::Packets::Add(packet, *nmsg);
            party_->WriteToMembers(*nmsg);
#ifdef DEBUG_GAME
            LOG_DEBUG << "Acceptor: " << id_ << ", Leader: " << playerId << ", Party: " << party_->GetId() << std::endl;
#endif
        }
        // else party maybe full
    }
}

void Player::CRQPartyRejectInvite(uint32_t inviterId)
{
    // We are the rejecter
    if (!AB::Entities::IsOutpost(GetGame()->data_.type))
        return;
    std::shared_ptr<Player> leader = GetSubsystem<PlayerManager>()->GetPlayerById(inviterId);
    if (leader)
    {
        if (leader->GetParty()->RemoveInvite(GetPtr<Player>()))
        {
            auto nmsg = Net::NetworkMessage::GetNew();
            nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyInviteRemoved);
            AB::Packets::Server::PartyPlayerRemoved packet = {
                inviterId,
                id_,
                leader->GetParty()->GetId()
            };
            AB::Packets::Add(packet, *nmsg);
            // Inform the party
            leader->GetParty()->WriteToMembers(*nmsg);
            // Inform us
            WriteToOutput(*nmsg);
        }
    }
}

void Player::CRQPartyGetMembers(uint32_t partyId)
{
    std::shared_ptr<Party> party = GetSubsystem<PartyManager>()->Get(partyId);
    if (party)
    {
        auto nmsg = Net::NetworkMessage::GetNew();
        nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyInfoMembers);
        size_t count = party->GetMemberCount();
        AB::Packets::Server::PartyMembersInfo packet;
        packet.partyId = partyId;
        packet.count = static_cast<uint8_t>(count);
        packet.members.reserve(count);

        // We also need invalid (i.e. not yet connected) members,
        // therefore we can not use Party::VisitMembers()
        const auto& members = party->GetMembers();
        for (const auto& m : members)
        {
            if (auto sm = m.lock())
                packet.members.push_back(sm->id_);
            else
                packet.members.push_back(0);
        }
        AB::Packets::Add(packet, *nmsg);
        WriteToOutput(*nmsg);
#ifdef DEBUG_GAME
        LOG_DEBUG << "Player: " << id_ << ", Party: " << partyId << ", Count: " << static_cast<int>(count) << std::endl;
#endif
    }
#ifdef DEBUG_GAME
    else
        LOG_DEBUG << "Party not found: " << partyId << std::endl;
#endif
}

void Player::CRQSetOnlineStatus(AB::Entities::OnlineStatus status)
{
    if (status == AB::Entities::OnlineStatusOffline || status == account_.onlineStatus)
        // This can not be set by the user
        return;

    account_.onlineStatus = status;
    auto* client = GetSubsystem<IO::DataClient>();
    client->Update(account_);
    GetGame()->BroadcastPlayerChanged(*this, AB::GameProtocol::PlayerInfoFieldOnlineStatus);
}

void Player::CRQSetSecondaryProfession(uint32_t profIndex)
{
    if (IsInOutpost())
    {
        auto nmsg = Net::NetworkMessage::GetNew();
        std::string current = skills_->prof2_.uuid;
        if (skills_->SetSecondaryProfession(profIndex))
        {
            // The player may have equipped skills from the previous secondary profession that are not
            // available anymore, so we must validate the whole skillbar and remove all skills from the
            // old secondary profession.
            for (int i = 0; i < PLAYER_MAX_SKILLS; ++i)
            {
                auto _skill = skills_->GetSkill(i);
                if (!_skill)
                    continue;

                if (_skill->data_.professionUuid.compare(current) == 0)
                {
                    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ObjectSetSkill);
                    AB::Packets::Server::ObjectSetSkill packet{
                        id_,
                        0,
                        static_cast<uint8_t>(i)
                    };
                    AB::Packets::Add(packet, *nmsg);
                }
            }
            WriteToOutput(*nmsg);
        }
    }

    // If it fails or not inform the client of the current profession
    AB::Packets::Server::ObjectSecProfessionChanged packet {
        id_,
        skills_->prof2_.index
    };
    auto& gameStatus = GetGame()->GetGameStatus();
    gameStatus.AddByte(AB::GameProtocol::ServerPacketType::ObjectSecProfessionChanged);
    AB::Packets::Add(packet, gameStatus);
}

void Player::CRQSetAttributeValue(uint32_t attribIndex, uint8_t value)
{
    if (attribIndex >= static_cast<uint32_t>(Attribute::__Last))
        return;

    Attribute index = static_cast<Attribute>(attribIndex);

    if (IsInOutpost() && value <= MAX_PLAYER_ATTRIBUTE_RANK)
        skills_->SetAttributeRank(index, value);

    uint32_t newValue = skills_->GetAttributeRank(index);
    int remaining = static_cast<int>(GetAttributePoints()) - skills_->GetUsedAttributePoints();
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ObjectSetAttributeValue);
    AB::Packets::Server::SetObjectAttributeValue packet {
        id_,
        attribIndex,
        static_cast<int8_t>(newValue),
        static_cast<uint8_t>(remaining)
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::CRQEquipSkill(uint32_t skillIndex, uint8_t pos)
{
    if (pos >= PLAYER_MAX_SKILLS)
    {
        LOG_WARNING << "Invalid skill position " << static_cast<int>(pos) << std::endl;
        return;
    }

    auto nmsg = Net::NetworkMessage::GetNew();
    if (IsInOutpost())
    {
        auto haveAccess = [&](const AB::Entities::Skill& skill, bool haveLocked)
        {
            if (AB::Entities::HasSkillAccess(skill, AB::Entities::SkillAccessPlayer))
                return true;
            if (haveLocked)
            {
                // This player can have GM locked skills
                if (AB::Entities::HasSkillAccess(skill, AB::Entities::SkillAccessGM))
                    return true;
            }
            return false;
        };

        auto validateSetSkill = [&](int pos, std::shared_ptr<Skill> skill) -> bool
        {
            if (skill)
            {
                for (int i = 0; i < PLAYER_MAX_SKILLS; ++i)
                {
                    auto _skill = skills_->GetSkill(i);
                    if (!_skill)
                        continue;

                    if (_skill->GetIndex() == skill->GetIndex())
                    {
                        skills_->RemoveSkill(i);
                        // No duplicate skills
                        nmsg->AddByte(AB::GameProtocol::ServerPacketType::ObjectSetSkill);
                        AB::Packets::Server::ObjectSetSkill packet{
                            id_,
                            0,
                            static_cast<uint8_t>(i)
                        };
                        AB::Packets::Add(packet, *nmsg);
                    }
                    if (skill->data_.isElite)
                    {
                        // Only one elite skill
                        if (_skill->data_.isElite && skill->data_.index != _skill->data_.index)
                        {
                            nmsg->AddByte(AB::GameProtocol::ServerPacketType::ObjectSetSkill);
                            AB::Packets::Server::ObjectSetSkill packet{
                                id_,
                                0,
                                static_cast<uint8_t>(i)
                            };
                            AB::Packets::Add(packet, *nmsg);
                        }
                    }
                }
            }
            return skills_->SetSkill(pos, skill);
        };

        if (skillIndex != 0)
        {
            auto* sm = GetSubsystem<SkillManager>();
            auto skill = sm->Get(skillIndex);
            if (skill)
            {
                if (haveAccess(skill->data_, account_.type >= AB::Entities::AccountTypeGamemaster))
                    validateSetSkill(static_cast<int>(pos), skill);
            }
            else
                LOG_WARNING << "No skill with index " << skillIndex << " found" << std::endl;
        }
        else
            skills_->RemoveSkill(static_cast<int>(pos));
    }
    // Always send a response, so the client can update its UI
    const uint32_t newIndex = skills_->GetIndexOfSkill(static_cast<int>(pos));
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ObjectSetSkill);
    AB::Packets::Server::ObjectSetSkill packet {
        id_,
        newIndex,
        pos
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::CRQLoadSkillTemplate(std::string templ)
{
    bool success = false;
    uint32_t oldProf = skills_->prof2_.index;

    if (IsInOutpost())
        success = skills_->Load(templ, account_.type >= AB::Entities::AccountTypeGamemaster);

    if (success && oldProf != skills_->prof2_.index)
    {
        // Loading a skill template may also change the secondary profession.
        // We need ot inform all players.
        AB::Packets::Server::ObjectSecProfessionChanged packet{
            id_,
            skills_->prof2_.index
        };
        auto& gameStatus = GetGame()->GetGameStatus();
        gameStatus.AddByte(AB::GameProtocol::ServerPacketType::ObjectSecProfessionChanged);
        AB::Packets::Add(packet, gameStatus);
    }

    const std::string newTempl = skills_->Encode();
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::PlayerSkillTemplLoaded);
    AB::Packets::Server::SkillTemplateLoaded packet{
        id_,
        newTempl
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}


bool Player::IsIgnored(const Player& player) const
{
    return GetFriendList().IsIgnored(player.account_.uuid);
}

bool Player::IsIgnored(const std::string& name) const
{
    return GetFriendList().IsIgnoredByName(name);
}

bool Player::IsFriend(const Player& player) const
{
    return GetFriendList().IsFriend(player.account_.uuid);
}

bool Player::IsOnline() const
{
    return AB::Entities::IsOnline(account_.onlineStatus);
}

Party* Player::_LuaGetParty()
{
    auto party = GetParty();
    return party ? party.get() : nullptr;
}

void Player::LoadFriendList()
{
    friendList_ = std::make_unique<FriendList>(data_.accountUuid);
    friendList_->Load();
}

void Player::OnHandleCommand(AB::GameProtocol::CommandType type,
    const std::string& arguments, Net::NetworkMessage& message)
{
    switch (type)
    {
    case AB::GameProtocol::CommandType::Unknown:
        break;
    case AB::GameProtocol::CommandType::Ip:
    case AB::GameProtocol::CommandType::PrefPath:
    case AB::GameProtocol::CommandType::Help:
    case AB::GameProtocol::CommandType::Quit:
        // Client side only
        break;
    case AB::GameProtocol::CommandType::ChatGeneral:
        HandleGeneralChatCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::ChatParty:
        HandlePartyChatCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Roll:
        HandleRollCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Sit:
        HandleSitCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Stand:
        HandleStandCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Cry:
        HandleCryCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Taunt:
        HandleTauntCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Ponder:
        HandlePonderCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Wave:
        HandleWaveCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Laugh:
        HandleLaughCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Age:
        HandleAgeCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Health:
        HandleHpCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Xp:
        HandleXpCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Pos:
        HandlePosCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::ChatWhisper:
        HandleWhisperCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::ChatGuild:
        HandleChatGuildCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::ChatTrade:
        HandleChatTradeCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Resign:
        HandleResignCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Stuck:
        HandleStuckCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::ServerId:
        HandleServerIdCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Die:
        HandleDieCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Deaths:
        HandleDeathsCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::Instances:
        HandleInstancesCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::GodMode:
        HandleGodModeCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::GMInfo:
        HandleGMInfoCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::EnterMap:
        HandleEnterMapCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::EnterInstance:
        HandleEnterInstanceCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandType::GotoPlayer:
        HandleGotoPlayerCommand(arguments, message);
        break;
    }
}

void Player::HandleServerIdCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGamemaster)
    {
        HandleUnknownCommand();
        return;
    }

    // Since it's more for debugging, it's only available for >= GM
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::ServerId),
        GetName(),
        Application::Instance->GetServerId()
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::HandleWhisperCommand(const std::string& arguments, Net::NetworkMessage&)
{
    size_t p = arguments.find(',');
    if (p == std::string::npos)
        return;

    const std::string name = arguments.substr(0, p);
    const std::string msg = sa::LeftTrim(arguments.substr(p + 1, std::string::npos));
    std::shared_ptr<Player> target = GetSubsystem<PlayerManager>()->GetPlayerByName(name);
    if (target)
    {
        // Found a player with the name so the target is on this server.
        std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Whisper, target->id_);
        if (channel)
        {
            if (channel->Talk(*this, msg))
            {
                auto nmsg = Net::NetworkMessage::GetNew();
                nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
                AB::Packets::Server::ServerMessage packet = {
                    static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::PlayerGotMessage),
                    name,
                    msg
                };
                AB::Packets::Add(packet, *nmsg);
                WriteToOutput(*nmsg);
                return;
            }
        }
    }

    // No player found with the name, pass the message to the message server
    AB_PROFILE;
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    AB::Entities::Character character;
    character.name = name;
    if (cli->Read(character) && (character.lastLogin > character.lastLogout))
    {
        // Is online
        AB::Entities::Account account;
        account.uuid = character.accountUuid;
        if (cli->Read(account) && AB::Entities::IsOnline(account.onlineStatus))
        {
            // The player may set his status to offline.
            std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Whisper, character.uuid);
            if (channel->Talk(*this, msg))
            {
                auto nmsg = Net::NetworkMessage::GetNew();
                nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
                AB::Packets::Server::ServerMessage packet = {
                    static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::PlayerGotMessage),
                    name,
                    msg
                };
                AB::Packets::Add(packet, *nmsg);
                WriteToOutput(*nmsg);
                return;
            }
        }
    }

    // Send not online message
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::PlayerNotOnline),
        GetName(),
        name
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::HandleChatGuildCommand(const std::string& arguments, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Guild, account_.guildUuid);
    if (channel)
        channel->Talk(*this, arguments);
}

void Player::HandleChatTradeCommand(const std::string& arguments, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Trade, 0);
    if (channel)
        channel->Talk(*this, arguments);
}

void Player::HandleResignCommand(const std::string&, Net::NetworkMessage& message)
{
    if (AB::Entities::IsOutpost(GetGame()->data_.type))
        return;
    message.AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::PlayerResigned),
        GetName(),
        ""
    };
    AB::Packets::Add(packet, message);
    resigned_ = true;
}

void Player::HandleStuckCommand(const std::string&, Net::NetworkMessage& message)
{
    message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectSetPosition);
    AB::Packets::Server::ObjectPosUpdate packet = {
        id_,
        {
            transformation_.position_.x_,
            transformation_.position_.y_,
            transformation_.position_.z_
        }
    };
    AB::Packets::Add(packet, message);
}

void Player::HandleAgeCommand(const std::string&, Net::NetworkMessage&)
{
    // In seconds
    const uint32_t playTime = static_cast<uint32_t>(data_.onlineTime) +
        static_cast<uint32_t>((Utils::Tick() - loginTime_) / 1000);
    // In seconds
    const uint32_t age = static_cast<uint32_t>((Utils::Tick() - data_.creation) / 1000);

    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::Age),
        GetName(),
        std::to_string(age) + ":" + std::to_string(playTime)
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::HandleHpCommand(const std::string&, Net::NetworkMessage&)
{
    auto nmsg = Net::NetworkMessage::GetNew();
    const int maxHp = resourceComp_->GetMaxHealth();
    const int hp = resourceComp_->GetHealth();
    const int maxE = resourceComp_->GetMaxEnergy();
    const int e = resourceComp_->GetEnergy();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::Hp),
        GetName(),
        std::to_string(hp) + ":" + std::to_string(maxHp) + "|" + std::to_string(e) + ":" + std::to_string(maxE)
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::HandleXpCommand(const std::string&, Net::NetworkMessage&)
{
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::Xp),
        GetName(),
        std::to_string(data_.xp) + "|" + std::to_string(data_.skillPoints)
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::HandlePosCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGamemaster)
    {
        HandleUnknownCommand();
        return;
    }

    auto nmsg = Net::NetworkMessage::GetNew();
    std::stringstream ss;
    ss << transformation_.position_.x_ << "," <<
        transformation_.position_.y_ << "," <<
        transformation_.position_.z_;
    ss << " " << transformation_.GetYRotation();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::Pos),
        GetName(),
        ss.str()
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::HandleRollCommand(const std::string& arguments, Net::NetworkMessage& message)
{
    if (!Utils::IsNumber(arguments))
        return;
    const int max = std::stoi(arguments);
    if (max < ROLL_MIN || max > ROLL_MAX)
        return;
    const int res = static_cast<int>(GetSubsystem<Crypto::Random>()->GetFloat() * static_cast<float>(max)) + 1;
    message.AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::Roll),
        GetName(),
        std::to_string(res) + ":" + std::to_string(max)
    };
    AB::Packets::Add(packet, message);
}

void Player::HandleSitCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureState::EmoteSit);
}

void Player::HandleStandCommand(const std::string&, Net::NetworkMessage&)
{
    if (stateComp_.GetState() == AB::GameProtocol::CreatureState::EmoteSit)
        stateComp_.SetState(AB::GameProtocol::CreatureState::Idle);
}

void Player::HandleCryCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureState::EmoteCry);
}

void Player::HandleTauntCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureState::EmoteTaunt);
}

void Player::HandlePonderCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureState::EmotePonder);
}

void Player::HandleWaveCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureState::EmoteWave);
}

void Player::HandleLaughCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureState::EmoteLaugh);
}

void Player::HandleDeathsCommand(const std::string&, Net::NetworkMessage&)
{
    // TODO:
}

void Player::HandleDieCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGod)
    {
        HandleUnknownCommand();
        return;
    }

    Die();
}

void Player::HandleInstancesCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGod)
    {
        HandleUnknownCommand();
        return;
    }

    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);

    auto* gameMan = GetSubsystem<GameManager>();
    const auto& games = gameMan->GetGames();
    std::stringstream ss;
    for (const auto& game : games)
    {
        ss << game.second->instanceData_.uuid << ",";
        ss << game.second->data_.uuid << ",";
        ss << game.second->data_.name << ";";
    }

    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::Instances),
        GetName(),
        ss.str()
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::HandleGeneralChatCommand(const std::string& arguments, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Map, GetGame()->id_);
    if (channel)
        channel->Talk(*this, arguments);
}

void Player::HandlePartyChatCommand(const std::string& arguments, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Party, GetParty()->GetId());
    if (channel)
        channel->Talk(*this, arguments);
}

void Player::HandleGodModeCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGamemaster)
    {
        HandleUnknownCommand();
        return;
    }

    static const uint32_t EFFECTINDEX_UNDESTROYABLE = 900000;

    if (effectsComp_->HasEffect(EFFECTINDEX_UNDESTROYABLE))
        effectsComp_->RemoveEffect(EFFECTINDEX_UNDESTROYABLE);
    else
        effectsComp_->AddEffect(std::shared_ptr<Actor>(), EFFECTINDEX_UNDESTROYABLE, 0);
}

void Player::HandleGMInfoCommand(const std::string& message, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGamemaster)
    {
        HandleUnknownCommand();
        return;
    }

    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::GMInfo),
        GetName(),
        message
    };
    AB::Packets::Add(packet, *nmsg);

    auto* playerMan = GetSubsystem<PlayerManager>();
    playerMan->VisitPlayers([&nmsg](Player& current) {
        current.WriteToOutput(*nmsg);
        return Iteration::Continue;
    });
}

void Player::HandleEnterMapCommand(const std::string& mapName, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGod)
    {
        HandleUnknownCommand();
        return;
    }

    std::string uuid = IO::IOGame::GetGameUuidFromName(mapName);
    if (Utils::Uuid::IsEmpty(uuid))
        return;
    ChangeMap(uuid);
}

void Player::HandleEnterInstanceCommand(const std::string& instanceUuid, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGod)
    {
        HandleUnknownCommand();
        return;
    }

    auto* gameMan = GetSubsystem<GameManager>();
    auto game = gameMan->GetInstance(instanceUuid);
    if (!game)
        return;

    ChangeInstance(game->data_.uuid, instanceUuid);
}

void Player::HandleGotoPlayerCommand(const std::string& playerName, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGamemaster)
    {
        HandleUnknownCommand();
        return;
    }

    // May need to enter this command twice:
    // 1. Change to the instance
    // 2. Teleport to player
    auto* playerMan = GetSubsystem<PlayerManager>();
    auto player = playerMan->GetPlayerByName(playerName);
    if (!player)
        return;

    auto* gameMan = GetSubsystem<GameManager>();
    const std::string& currentMap = player->data_.currentMapUuid;
    const std::string& currentInst = player->data_.instanceUuid;
    if (!gameMan->InstanceExists(currentInst))
        return;

    if (Utils::Uuid::IsEqual(data_.instanceUuid, currentInst))
    {
        // This is the same instance -> teleport to player
        moveComp_->SetPosition(player->transformation_.position_);
        moveComp_->forcePosition_ = true;
        return;
    }
    // enter the same instance as the player
    ChangeInstance(currentMap, currentInst);
}

void Player::HandleUnknownCommand()
{
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::UnknownCommand),
        GetName(),
        ""
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::ChangeMap(const std::string& mapUuid)
{
    // mapUuid No reference

    // If we are the leader tell all members to change the instance.
    // If not, tell the leader to change the instance.
    auto party = GetParty();
    if (!party)
        return;

    if (!IsInOutpost())
    {
        // The player leaves the party and changes the instance
        PartyLeave();
        party = GetParty();
    }
    if (party->IsLeader(*this))
        // If we are the leader tell all members to change the instance.
        party->ChangeInstance(mapUuid);
}

void Player::ChangeServerInstance(const std::string& serverUuid, const std::string& mapUuid, const std::string& instanceUuid)
{
    resigned_ = false;
    if (client_)
        return client_->ChangeServerInstance(serverUuid, mapUuid, instanceUuid);
    LOG_ERROR << "client_ = null" << std::endl;
}

void Player::CRQQueueForMatch()
{
    assert(GetParty());
    if (!GetParty()->IsLeader(*this))
        return;

    auto game = GetGame();
    assert(game);
    if (Utils::Uuid::IsEmpty(game->data_.queueMapUuid))
        return;

    auto* client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::QueueAdd;
    IO::PropWriteStream stream;
    stream.WriteString(data_.uuid);
    stream.WriteString(game->data_.queueMapUuid);
    msg.SetPropStream(stream);
    client->Write(msg);
    queueing_ = true;
}

void Player::CRQUnqueueForMatch()
{
    assert(GetParty());
    if (!GetParty()->IsLeader(*this))
        return;

    auto* client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::QueueRemove;
    IO::PropWriteStream stream;
    stream.WriteString(data_.uuid);
    msg.SetPropStream(stream);
    client->Write(msg);
    queueing_ = false;
}

void Player::CRQDeleteQuest(uint32_t index)
{
    if (!IsInOutpost())
        return;

    questComp_->DeleteQuest(index);
}

void Player::CRQHasQuests(uint32_t npcId)
{
    auto* npc = GetGame()->GetObject<Npc>(npcId);
    if (!npc)
        return;

    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::QuestNpcHasQuest);
    AB::Packets::Server::NpcHasQuest packet = {
        npcId,
        npc->HaveQuestsForPlayer(*this)
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}


void Player::ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid)
{
    resigned_ = false;
    if (client_)
        return client_->ChangeInstance(mapUuid, instanceUuid);
    LOG_ERROR << "client_ = null" << std::endl;
}

}
