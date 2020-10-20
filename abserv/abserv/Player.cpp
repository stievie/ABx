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

#include "Player.h"
#include "Application.h"
#include "Chat.h"
#include "FriendList.h"
#include "GameManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "InteractionComp.h"
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
#include "PlayerManager.h"
#include "ProtocolGame.h"
#include "QuestComp.h"
#include "SkillManager.h"
#include "TradeComp.h"
#include <AB/Entities/AccountItemList.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/CraftableItemList.h>
#include <AB/Entities/Item.h>
#include <AB/Entities/MerchantItemList.h>
#include <AB/Entities/PlayerItemList.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <AB/ProtocolCodes.h>
#include <abscommon/MessageClient.h>
#include <abscommon/StringUtils.h>
#include <abshared/SkillsHelper.h>
#include <abshared/Attributes.h>
#include <sa/Assert.h>
#include <sa/StringTempl.h>
#include <sa/Transaction.h>

#define DEBUG_GAME

namespace Game {

void Player::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Actor>()
        .addFunction("GetParty", &Player::_LuaGetParty)
        .addFunction("ChangeMap", &Player::ChangeMap)
        .addFunction("TriggerDialog", &Player::TriggerDialog)
        .addFunction("AddQuest", &Player::AddQuest)
        .addFunction("GetQuestReward", &Player::GetQuestReward)
        .addFunction("SatisfyQuestRequirements", &Player::SatisfyQuestRequirements)
    );
    // clang-format on
}

void Player::PlayerError(AB::GameProtocol::PlayerErrorValue error, Net::NetworkMessage& message)
{
    message.AddByte(AB::GameProtocol::ServerPacketType::PlayerError);
    AB::Packets::Server::PlayerError packet = {
        static_cast<uint8_t>(error)
    };
    AB::Packets::Add(packet, message);
}

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    Actor(),
    client_(client),
    questComp_(ea::make_unique<Components::QuestComp>(*this)),
    tradeComp_(ea::make_unique<Components::TradeComp>(*this)),
    interactionComp_(ea::make_unique<Components::InteractionComp>(*this))
{
    events_.Subscribe<void(AB::GameProtocol::CommandType, const std::string&, Net::NetworkMessage&)>(EVENT_ON_HANDLECOMMAND,
        std::bind(&Player::OnHandleCommand, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    events_.Subscribe<void(void)>(EVENT_ON_INVENTORYFULL, std::bind(&Player::OnInventoryFull, this));
    events_.Subscribe<void(void)>(EVENT_ON_CHESTFULL, std::bind(&Player::OnChestFull, this));
    events_.Subscribe<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
        std::bind(
            &Player::OnPingObject, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

Player::~Player() = default;

void Player::SendError(AB::GameProtocol::PlayerErrorValue value)
{
    auto msg = Net::NetworkMessage::GetNew();
    PlayerError(value, *msg);
    WriteToOutput(*msg);
}

void Player::SetGame(ea::shared_ptr<Game> game)
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
        account_.type >= AB::Entities::AccountType::Gamemaster;
}

bool Player::CanUseSkill() const
{
    return !AB::Entities::IsOutpost(GetGame()->data_.type) ||
        account_.type >= AB::Entities::AccountType::Gamemaster;
}

void Player::SetLevel(uint32_t value)
{
    data_.level = static_cast<uint8_t>(value);
    resourceComp_->UpdateResources();
}

void Player::AddXp(int value)
{
    ASSERT(value >= 0);
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

void Player::Logout(bool leavePary)
{
    // Maybe called from Main Thread
#ifndef DEBUG_GAME
    LOG_DEBUG << "Player logging out " << GetName() << std::endl;
#endif // DEBUG_GAME
    if (queueing_)
        CRQUnqueueForMatch();
    if (leavePary)
        PartyLeave();
    if (auto g = GetGame())
    {
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(std::bind(&Game::PlayerLeave, g, id_)));
    }
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
    // This is also called when changing an instance, so we want to stay in the party.
    Logout(false);
}

void Player::CRQPing(int64_t clientTick)
{
    lastPing_ = sa::time::tick();
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::GamePong);
    // Depending on the timezone of server and client the server may also be behind, i.e. difference is negative.
    AB::Packets::Server::GamePong packet = { static_cast<int32_t>(sa::time::tick() - clientTick) };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::TriggerDialog(uint32_t triggererId, uint32_t dialogIndex)
{
    if (triggererId != 0)
    {
        auto* object = GetGame()->GetObject<GameObject>(triggererId);
        if (!object)
            return;
        float dist = GetDistance(object);
        if (dist > RANGE_ADJECENT)
            return;
    }
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::DialogTrigger);
    AB::Packets::Server::DialogTrigger packet = {
        triggererId,
        dialogIndex
    };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::TriggerQuestSelectionDialog(uint32_t triggererId, const ea::set<uint32_t>& quests)
{
    if (triggererId != 0)
    {
        auto* object = GetGame()->GetObject<GameObject>(triggererId);
        if (!object)
            return;
        float dist = GetDistance(object);
        if (dist > RANGE_ADJECENT)
            return;
    }

    if (quests.size() == 0)
        return;
    if (quests.size() == 1)
    {
        TriggerQuestDialog(triggererId, *quests.begin());
        return;
    }
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::QuestSelectionDialogTrigger);
    AB::Packets::Server::QuestSelectionDialogTrigger packet;
    packet.triggererId = triggererId;
    packet.count = static_cast<uint8_t>(quests.size());
    packet.quests.reserve(packet.count);
    for (auto i : quests)
        packet.quests.push_back(i);
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::TriggerQuestDialog(uint32_t triggererId, uint32_t index)
{
    if (triggererId != 0)
    {
        auto* object = GetGame()->GetObject<GameObject>(triggererId);
        if (!object)
            return;
        float dist = GetDistance(object);
        if (dist > RANGE_ADJECENT)
            return;
    }

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::QuestDialogTrigger);
    AB::Packets::Server::QuestDialogTrigger packet = {
        triggererId,
        index
    };
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::TriggerTradeDialog(uint32_t targetId)
{
    // We initiated the trade operation so we are the source id
    if (targetId == 0)
        return;

    auto* target = GetGame()->GetObject<Player>(targetId);
    if (!target)
        return;
    const float dist = GetDistance(target);
    if (dist > RANGE_ADJECENT)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::TradeDialogTrigger);
    AB::Packets::Server::TradeDialogTrigger packet = {
        id_,
        targetId
    };
    AB::Packets::Add(packet, *msg);
    // Send both parties the message
    WriteToOutput(*msg);
    target->WriteToOutput(*msg);
}

MailBox& Player::GetMailBox()
{
    if (!mailBox_)
        mailBox_ = ea::make_unique<MailBox>(data_.accountUuid);
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
    packet.maxMoney = static_cast<uint32_t>(inventoryComp_->GetMaxInventoryMoney());
    packet.maxItems = static_cast<uint32_t>(inventoryComp_->GetInventorySize());
    packet.count = static_cast<uint16_t>(count);

    inventoryComp_->VisitInventory([&packet](const Item& current)
    {
        packet.items.push_back({
            current.data_.index,
            static_cast<uint16_t>(current.data_.type),
            current.concreteItem_.count,
            current.concreteItem_.value,
            current.stats_.ToString(),
            static_cast<uint8_t>(current.concreteItem_.storagePlace),
            current.concreteItem_.storagePos,
            current.concreteItem_.flags
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

void Player::CRQSetItemPos(AB::Entities::StoragePlace currentPlace,
    uint16_t currentPos, AB::Entities::StoragePlace newPlace, uint16_t newPos,
    uint32_t count)
{
    (void)count;
//    LOG_INFO << "CRQSetItemPos(): place: " << static_cast<int>(currentPlace) << " pos: " << currentPos <<
//        " new place: " << static_cast<int>(newPlace) << " new pos: " << newPos << std::endl;
    if (currentPlace == AB::Entities::StoragePlace::Inventory && tradeComp_->IsTrading())
    {
        // Because we use the item position to identify the item that is traded, we don't allow
        // to reorder the items while trading.
        auto msg = Net::NetworkMessage::GetNew();
        PlayerError(AB::GameProtocol::PlayerErrorValue::NotAllowedWhileTrading, *msg);
        WriteToOutput(*msg);
        return;
    }

    if (newPlace == AB::Entities::StoragePlace::Chest)
    {
        if (inventoryComp_->IsChestFull())
        {
            OnChestFull();
            return;
        }
        if (inventoryComp_->GetChestItem(newPos) != nullptr)
        {
            // TODO: If there is already some item, exchange it
            return;
        }
    }
    else if (newPlace == AB::Entities::StoragePlace::Inventory)
    {
        if (inventoryComp_->IsInventoryFull())
        {
            OnInventoryFull();
            return;
        }
        if (inventoryComp_->GetInventoryItem(newPos) != nullptr)
        {
            // TODO: If there is already some item, exchange it
            return;
        }
    }
    else
    {
        //TODO: Maybe another storage place is possible, e.g. as Mail attachment.
        return;
    }

    if (currentPlace != AB::Entities::StoragePlace::Chest && currentPlace != AB::Entities::StoragePlace::Inventory)
        return;

    // TODO: If count != whole stack split items
    auto removeItem = [&]() -> uint32_t
    {
        if (currentPlace == AB::Entities::StoragePlace::Inventory)
            return inventoryComp_->RemoveInventoryItem(currentPos);
        return inventoryComp_->RemoveChestItem(currentPos);
    };

    uint32_t itemId = removeItem();
    if (itemId == 0)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    // Remove from current
    if (currentPlace == AB::Entities::StoragePlace::Chest)
        msg->AddByte(AB::GameProtocol::ServerPacketType::ChestItemDelete);
    else
        msg->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
    AB::Packets::Server::InventoryItemDelete packet = {
        currentPos
    };
    AB::Packets::Add(packet, *msg);

    // Add to new
    if (newPlace == AB::Entities::StoragePlace::Chest)
        inventoryComp_->SetChestItem(itemId, msg.get(), newPos);
    else
        inventoryComp_->SetInventoryItem(itemId, msg.get(), newPos);
    WriteToOutput(*msg);
}

void Player::CRQDropInventoryItem(uint16_t pos, uint32_t count)
{
    if (GetGame()->data_.type == AB::Entities::GameTypePvPCombat)
        return;

    auto* item = inventoryComp_->GetInventoryItem(pos);
    if (!item)
    {
        LOG_ERROR << "No item at pos " << static_cast<int>(pos) << " in inventory" << std::endl;
        return;
    }
    if (item->concreteItem_.count < count)
    {
        LOG_WARNING << "CHEAT: Player " << GetName() << " tries to drop more items than available. Available " <<
            item->concreteItem_.count << " count " << count << std::endl;
        return;
    }

    auto msg = Net::NetworkMessage::GetNew();
    if (count == item->concreteItem_.count)
    {
        // Drop the whole stack
        inventoryComp_->RemoveInventoryItem(pos);
        item->concreteItem_.storagePlace = AB::Entities::StoragePlace::Scene;
        item->concreteItem_.storagePos = 0;
        auto rng = GetSubsystem<Crypto::Random>();
        ea::shared_ptr<ItemDrop> drop = ea::make_shared<ItemDrop>(item->id_);
        drop->transformation_.position_ = transformation_.position_;
        // Random pos around dropper
        drop->transformation_.position_.y_ += 0.2f;
        drop->transformation_.position_.x_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
        drop->transformation_.position_.z_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
        drop->SetSource(GetPtr<Actor>());
        GetGame()->SpawnItemDrop(drop);

        msg->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
        AB::Packets::Server::InventoryItemDelete packet{
            pos
        };
        AB::Packets::Add(packet, *msg);
    }
    else
    {
        // Split it
        auto* newItem = inventoryComp_->SplitStack(item, count, AB::Entities::StoragePlace::Scene, 0);
        if (!newItem)
            return;
        auto rng = GetSubsystem<Crypto::Random>();
        ea::shared_ptr<ItemDrop> drop = ea::make_shared<ItemDrop>(newItem->id_);
        drop->transformation_.position_ = transformation_.position_;
        // Random pos around dropper
        drop->transformation_.position_.y_ += 0.2f;
        drop->transformation_.position_.x_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
        drop->transformation_.position_.z_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
        drop->SetSource(GetPtr<Actor>());
        GetGame()->SpawnItemDrop(drop);
        Components::InventoryComp::WriteItemUpdate(item, msg.get());
    }
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
    packet.maxMoney = static_cast<uint32_t>(inventoryComp_->GetMaxChestMoney());
    packet.maxItems = static_cast<uint32_t>(inventoryComp_->GetChestSize());
    packet.count = static_cast<uint16_t>(count);
    packet.items.reserve(packet.count);
    inventoryComp_->VisitChest([&packet](const Item& current)
    {
        packet.items.push_back({
            current.data_.index,
            static_cast<uint16_t>(current.data_.type),
            current.concreteItem_.count,
            current.concreteItem_.value,
            current.stats_.ToString(),
            static_cast<uint8_t>(current.concreteItem_.storagePlace),
            current.concreteItem_.storagePos,
            current.concreteItem_.flags
        });
        return Iteration::Continue;
    });
    AB::Packets::Add(packet, *msg);

    WriteToOutput(*msg);
}

void Player::CRQDestroyChestItem(uint16_t pos)
{
    if (!inventoryComp_->DestroyChestItem(pos))
        return;

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

    auto& fl = GetFriendList();
    if (!fl.GetFriendByAccount(accountUuid, f))
        return;

    auto res = fl.Remove(accountUuid);

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
    if (f.relation == AB::Entities::FriendRelationUnknown)
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

    if (!friendList_)
        LoadFriendList();
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
    if (!IO::IOPlayer::GetPlayerInfoByAccount(accountUuid, ch))
        // If there is no such thing, we just don't reply to this request
        return;
    SendPlayerInfo(ch, fields);
}

void Player::CRQGetPlayerInfoByName(const std::string name, uint32_t fields)
{
    AB::Entities::Character ch;
    if (!IO::IOPlayer::GetPlayerInfoByName(name, ch))
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
    msg->AddByte(AB::GameProtocol::ServerPacketType::ObjectPingTarget);
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
    PlayerError(AB::GameProtocol::PlayerErrorValue::InventoryFull, *msg);
    WriteToOutput(*msg);
}

void Player::OnChestFull()
{
    auto msg = Net::NetworkMessage::GetNew();
    PlayerError(AB::GameProtocol::PlayerErrorValue::ChestFull, *msg);
    WriteToOutput(*msg);
}

void Player::SetParty(ea::shared_ptr<Party> party)
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
        if (auto game = game_.lock())
            party_->SetPartySize(game->data_.partySize);
        data_.partyUuid = party_->data_.uuid;
        SetGroupId(party_->GetId());
    }
    party_->SetPlayer(GetPtr<Player>());
}

void Player::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    Actor::Update(timeElapsed, message);
    interactionComp_->Update(timeElapsed);
    tradeComp_->Update(timeElapsed);
    questComp_->Update(timeElapsed);
    auto party = GetParty();
    if (party->IsLeader(*this))
        party->Update(timeElapsed, message);
    questComp_->Write(message);
}

bool Player::RemoveMoney(uint32_t count)
{
    auto msg = Net::NetworkMessage::GetNew();
    const uint32_t amount = inventoryComp_->RemoveChestMoney(count, msg.get());
    WriteToOutput(*msg);
    return amount != 0;
}

bool Player::AddMoney(uint32_t count)
{
    auto msg = Net::NetworkMessage::GetNew();
    const uint32_t amount = inventoryComp_->AddInventoryMoney(count, msg.get());
    WriteToOutput(*msg);
    return amount != 0;
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
    WriteToOutput(*msg);
    return ret;
}

void Player::CRQDepositMoney(uint32_t amount)
{
    if (amount > inventoryComp_->GetInventoryMoney())
        return;

    auto msg = Net::NetworkMessage::GetNew();
    inventoryComp_->DepositMoney(amount, msg.get());
    WriteToOutput(*msg);
}

void Player::CRQWithdrawMoney(uint32_t amount)
{
    if (amount > inventoryComp_->GetChestMoney())
        return;

    auto msg = Net::NetworkMessage::GetNew();
    inventoryComp_->WithdrawMoney(amount, msg.get());
    WriteToOutput(*msg);
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
    ea::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (!player)
        return;

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

    ea::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
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
        player->SetParty(ea::shared_ptr<Party>());
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
        SetParty(ea::shared_ptr<Party>());
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

    ea::shared_ptr<Player> leader = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (!leader)
        return;

    // Leave current party
    PartyLeave();
    if (leader->GetParty()->AddPlayer(GetPtr<Player>()))
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
        LOG_DEBUG << "Acceptor: " << id_ << ", Leader: " << playerId << ", Party: " << party_->GetId() <<
            " Member count " << party_->GetMemberCount() << std::endl;
#endif
    }
    // else party maybe full
}

void Player::CRQPartyRejectInvite(uint32_t inviterId)
{
    // We are the rejecter
    if (!AB::Entities::IsOutpost(GetGame()->data_.type))
        return;
    ea::shared_ptr<Player> leader = GetSubsystem<PlayerManager>()->GetPlayerById(inviterId);
    if (!leader)
        return;

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

void Player::CRQPartyGetMembers(uint32_t partyId)
{
    ea::shared_ptr<Party> party = GetSubsystem<PartyManager>()->Get(partyId);
    if (!party)
    {
#ifdef DEBUG_GAME
        LOG_DEBUG << "Party not found: " << partyId << std::endl;
#endif
        return;
    }

    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::PartyMembersInfo);
    // Valid + not connected members
    size_t count = party->GetAllMemberCount();
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
    if (skills_->prof2_.index == profIndex)
        return;

    if (IsInOutpost())
    {
        auto nmsg = Net::NetworkMessage::GetNew();
        const std::string oldProf = skills_->prof2_.uuid;
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

                if (_skill->data_.professionUuid.compare(oldProf) == 0)
                {
                    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ObjectSetSkill);
                    AB::Packets::Server::ObjectSetSkill packet{
                        id_,
                        0,
                        static_cast<uint8_t>(i)
                    };
                    AB::Packets::Add(packet, *nmsg);
                    skills_->RemoveSkill(i);
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
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::SetObjectAttributeValue);
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

        auto professionsMatch = [&](const AB::Entities::Skill& skill)
        {
            return SkillProfessionMatches(skill, skills_->prof1_, &skills_->prof2_);
        };

        auto validateSetSkill = [&](int pos, ea::shared_ptr<Skill> skill) -> bool
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
                        skills_->RemoveSkill(i);
                        continue;
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
                            skills_->RemoveSkill(i);
                            continue;
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
                if (haveAccess(skill->data_, account_.type >= AB::Entities::AccountType::Gamemaster) &&
                    professionsMatch(skill->data_))
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
        success = skills_->Load(templ, account_.type >= AB::Entities::AccountType::Gamemaster);

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
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::SkillTemplateLoaded);
    AB::Packets::Server::SkillTemplateLoaded packet{
        id_,
        newTempl
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::CRQTradeRequest(uint32_t targetId)
{
    auto* target = GetGame()->GetObject<Player>(targetId);
    if (!target)
        return;

    auto error = tradeComp_->TradeWith(target->GetPtr<Player>());
    if (error != Components::TradeComp::TradeError::None)
    {
        auto msg = Net::NetworkMessage::GetNew();
        Components::TradeComp::WriteError(error, *msg);
        WriteToOutput(*msg);
    }
}

void Player::CRQTradeCancel()
{
    if (tradeComp_->IsTrading())
        tradeComp_->Cancel();
}

void Player::CRQTradeOffer(uint32_t money, std::vector<std::pair<uint16_t, uint32_t>> items)
{
    // We offer our trade partner the given items in our inventory
    if (!tradeComp_->IsTrading())
        return;
    tradeComp_->Offer(money, std::move(items));
}

void Player::CRQTradeAccept()
{
    if (!tradeComp_->IsTrading())
        return;
    tradeComp_->Accept();
}

void Player::CRQSellItem(uint32_t npcId, uint16_t pos, uint32_t count)
{
    auto* npc = GetGame()->GetObject<Npc>(npcId);
    if (!npc)
    {
        LOG_ERROR << "No NPC with id " << npcId << std::endl;
        return;
    }

    auto* item = inventoryComp_->GetInventoryItem(pos);
    if (!item)
        return;

    uint32_t price = item->concreteItem_.value;
    if (!AB::Entities::IsItemCustomized(item->concreteItem_.flags))
    {
        const auto it = calculatedItemPrices_.find(item->data_.uuid);
        if (it != calculatedItemPrices_.end())
            price = (*it).second.priceBuy;
    }

    auto msg = Net::NetworkMessage::GetNew();
    bool ret = inventoryComp_->SellItem(pos, count, price, msg.get());

    if (ret)
    {
        AB::Entities::InventoryItems inv;
        inv.uuid = data_.uuid;
        IO::DataClient* cli = GetSubsystem<IO::DataClient>();
        cli->Invalidate(inv);
    }

    WriteToOutput(*msg);
}

void Player::CRQBuyItem(uint32_t npcId, uint32_t id, uint32_t count)
{
    if (count == 0 || count > MAX_INVENTORY_STACK_SIZE)
        return;
    auto* npc = GetGame()->GetObject<Npc>(npcId);
    if (!npc)
        return;
    if (!IsInRange(Ranges::Adjecent, npc))
        return;

    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(id);
    if (!item)
    {
        LOG_ERROR << "Item " << id << " not in cache" << std::endl;
        return;
    }
    if (item->concreteItem_.storagePlace != AB::Entities::StoragePlace::Merchant)
    {
        LOG_WARNING << "CHEAT: Player " << GetName() << " wants to buy an item which does not belong to the merchant" << std::endl;
        return;
    }

    const auto it = calculatedItemPrices_.find(item->data_.uuid);
    if (it == calculatedItemPrices_.end())
    {
        LOG_ERROR << "Item " << item->data_.uuid << " not in price cache" << std::endl;
        return;
    }

    uint32_t price = (*it).second.priceSell;
    auto msg = Net::NetworkMessage::GetNew();
    if (item->concreteItem_.count < count)
        count = item->concreteItem_.count;

    if (price * count <= inventoryComp_->GetInventoryMoney())
    {
        if (inventoryComp_->BuyItem(item, count, price, msg.get()))
        {
            AB::Entities::InventoryItems inv;
            inv.uuid = data_.uuid;
            IO::DataClient* cli = GetSubsystem<IO::DataClient>();
            cli->Invalidate(inv);
        }
    }
    else
    {
        PlayerError(AB::GameProtocol::PlayerErrorValue::NotEnoughMoney, *msg);
    }

    WriteToOutput(*msg);
}

void Player::CRQGetMerchantItems(uint32_t npcId, AB::Entities::ItemType itemType, std::string searchName, uint8_t page)
{
    auto* npc = GetGame()->GetObject<Npc>(npcId);
    if (!npc)
        return;
    if (!IsInRange(Ranges::Adjecent, npc))
        return;
    if (page == 0)
        return;
    if (itemType != AB::Entities::ItemType::Unknown && !npc->IsSellingItemType(itemType))
        return;

    calculatedItemPrices_.clear();

    auto* cli = GetSubsystem<IO::DataClient>();
    AB::Entities::MerchantItemList ml;
    if (!cli->Read(ml))
    {
        LOG_ERROR << "Error reading merchant item list" << std::endl;
        return;
    }

    auto* factory = GetSubsystem<ItemFactory>();
    ea::set<AB::Entities::ItemType> availTypes;
    const bool doSearch = !searchName.empty();
    const std::string search = "*" + searchName + "*";
    ea::vector<size_t> itemIndices;
    itemIndices.reserve(ml.items.size());
    size_t index = 0;
    for (auto it = ml.items.begin(); it != ml.items.end(); ++it, ++index)
    {
        if (!npc->IsSellingItemType(it->type))
            continue;
        if (!npc->IsSellingItem(it->index))
            continue;
        availTypes.emplace(it->type);
        if (itemType != AB::Entities::ItemType::Unknown)
        {
            if (it->type != itemType)
                continue;
        }
        if (doSearch)
        {
            if (!sa::PatternMatch(it->name, search))
                continue;
        }
        itemIndices.push_back(index);
    }

    const size_t offset = static_cast<size_t>(page - 1) * MERCHANTITEMS_PAGESIZE;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::MerchantItems);
    AB::Packets::Server::MerchantItems packet;

    const size_t pageCount = (itemIndices.size() + MERCHANTITEMS_PAGESIZE - 1) / MERCHANTITEMS_PAGESIZE;
    uint16_t count = 0;
    if (offset < itemIndices.size())
    {
        auto it = itemIndices.begin();
        std::advance(it, offset);
        if (it != itemIndices.end())
        {
            auto* cache = GetSubsystem<ItemsCache>();
            for (; it != itemIndices.end(); ++it)
            {
                const auto& listItem = ml.items.at(*it);
                AB::Entities::ItemPrice price;
                price.uuid = listItem.itemUuid;
                if (!cli->Read(price))
                {
                    LOG_ERROR << "Error reading item price for " << listItem.itemUuid << std::endl;
                    continue;
                }

                uint32_t itemId = factory->GetConcreteId(listItem.concreteUuid);
                auto* item = cache->Get(itemId);
                // I guess this shouldn't happen
                ASSERT(item);

                if (item->concreteItem_.count == 0)
                    continue;

                ++count;
                AB::Packets::Server::Internal::MerchantItem merchantItem;
                merchantItem.id = itemId;
                merchantItem.index = item->data_.index;
                merchantItem.type = static_cast<uint16_t>(item->data_.type);
                // The player doesn't need to know how many items we have
                merchantItem.count = 0;
                merchantItem.value = item->concreteItem_.value;
                merchantItem.stats = item->stats_.ToString();
                merchantItem.flags = item->data_.itemFlags;
                merchantItem.buyPrice = price.priceBuy;
                merchantItem.sellPrice = price.priceSell;

                packet.items.push_back(std::move(merchantItem));

                calculatedItemPrices_.emplace(listItem.itemUuid, std::move(price));

                if (count >= MERCHANTITEMS_PAGESIZE)
                    break;
            }
        }
    }

    for (auto it = availTypes.rbegin(); it != availTypes.rend(); ++it)
    {
        packet.types.push_back(static_cast<uint16_t>(*it));
    }
    packet.typesCount = static_cast<uint16_t>(packet.types.size());

    packet.page = page;
    packet.pageCount = static_cast<uint8_t>(pageCount);
    packet.count = count;
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::CRQGetItemPrice(std::vector<uint16_t> items)
{
    if (items.size() == 0)
        return;

    auto* cli = GetSubsystem<IO::DataClient>();
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::ItemPrice);

    AB::Packets::Server::ItemPrice packet;

    for (auto pos : items)
    {
        Item* item = inventoryComp_->GetInventoryItem(pos);
        if (!item)
            continue;

        if (!AB::Entities::IsItemCustomized(item->concreteItem_.flags))
        {

            const auto it = calculatedItemPrices_.find(item->data_.uuid);
            if (it != calculatedItemPrices_.end())
            {
                packet.items.push_back({ pos, (*it).second.priceBuy });
                continue;
            }

            AB::Entities::ItemPrice price;
            price.uuid = item->data_.uuid;
            if (!cli->Read(price))
            {
                LOG_ERROR << "Error reading item price for " << item->data_.uuid << std::endl;
                continue;
            }

            packet.items.push_back({ pos, price.priceBuy });
            calculatedItemPrices_.emplace(item->data_.uuid, std::move(price));
        }
        else
        {
            // If the item is customized the merchant pays only the value
            packet.items.push_back({ pos, item->concreteItem_.value });
        }
    }

    if (packet.items.size() == 0)
        return;

    packet.count = static_cast<uint8_t>(packet.items.size());
    AB::Packets::Add(packet, *msg);

    WriteToOutput(*msg);
}

void Player::CRQGetCraftsmanItems(uint32_t npcId, AB::Entities::ItemType itemType, std::string searchName, uint8_t page)
{
    auto* npc = GetGame()->GetObject<Npc>(npcId);
    if (!npc)
        return;
    if (!IsInRange(Ranges::Adjecent, npc))
        return;
    if (page == 0)
        return;
    if (itemType != AB::Entities::ItemType::Unknown && !npc->IsSellingItemType(itemType))
        return;

    auto* cli = GetSubsystem<IO::DataClient>();
    AB::Entities::CraftableItemList ml;
    if (!cli->Read(ml))
    {
        LOG_ERROR << "Error reading craftable item list" << std::endl;
        return;
    }

    ea::set<AB::Entities::ItemType> availTypes;
    const bool doSearch = !searchName.empty();
    const std::string search = "*" + searchName + "*";
    ea::vector<size_t> itemIndices;
    itemIndices.reserve(ml.items.size());
    size_t index = 0;
    for (auto it = ml.items.begin(); it != ml.items.end(); ++it, ++index)
    {
        if (!npc->IsSellingItemType(it->type))
            continue;
        if (!npc->IsSellingItem(it->index))
            continue;
        availTypes.emplace(it->type);
        if (itemType != AB::Entities::ItemType::Unknown)
        {
            if (it->type != itemType)
                continue;
        }
        if (doSearch)
        {
            if (!sa::PatternMatch(it->name, search))
                continue;
        }
        itemIndices.push_back(index);
    }

    const size_t offset = static_cast<size_t>(page - 1) * MERCHANTITEMS_PAGESIZE;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ServerPacketType::CraftsmanItems);
    AB::Packets::Server::CraftsmanItems packet;

    const size_t pageCount = (itemIndices.size() + MERCHANTITEMS_PAGESIZE - 1) / MERCHANTITEMS_PAGESIZE;
    uint16_t count = 0;
    if (offset < itemIndices.size())
    {
        auto it = itemIndices.begin();
        std::advance(it, offset);
        if (it != itemIndices.end())
        {
            auto* factory = GetSubsystem<ItemFactory>();
            for (; it != itemIndices.end(); ++it)
            {
                const auto& listItem = ml.items.at(*it);

                ++count;
                AB::Packets::Server::Internal::Item craftsmanItem;
                craftsmanItem.index = listItem.index;
                craftsmanItem.type = static_cast<uint16_t>(listItem.type);
                craftsmanItem.count = 0;
                craftsmanItem.value = listItem.value;
                const auto possibleAttribs = GetPossibleItemAttributes(listItem.type);
                int attribIndex = possibleAttribs.size() == 1 ? static_cast<int>(*possibleAttribs.begin()) : static_cast<int>(Attribute::None);
                int damageType = possibleAttribs.size() == 1 ? -1 : static_cast<int>(DamageType::Unknown);
                // How much is to pay is stored in stats, and generatd when the concrete item is created (e.g. when dropped)
                craftsmanItem.stats = factory->GetMaxItemStats(listItem.uuid, npc->GetLevel(),
                    attribIndex, -1, damageType);
                craftsmanItem.flags = listItem.itemFlags;

                packet.items.push_back(std::move(craftsmanItem));

                if (count >= MERCHANTITEMS_PAGESIZE)
                    break;
            }
        }
    }

    for (auto it = availTypes.rbegin(); it != availTypes.rend(); ++it)
    {
        packet.types.push_back(static_cast<uint16_t>(*it));
    }
    packet.typesCount = static_cast<uint16_t>(packet.types.size());

    packet.page = page;
    packet.pageCount = static_cast<uint8_t>(pageCount);
    packet.count = count;
    AB::Packets::Add(packet, *msg);
    WriteToOutput(*msg);
}

void Player::CRQCraftItem(uint32_t npcId, uint32_t index, uint32_t count, uint32_t attributeIndex)
{
    if (count == 0 || count > MAX_INVENTORY_STACK_SIZE)
        return;
    if (attributeIndex >= static_cast<uint32_t>(Attribute::__Last) &&
        attributeIndex != static_cast<uint32_t>(Attribute::None))
        return;

    if (index == 0 || count == 0)
        return;

    auto* npc = GetGame()->GetObject<Npc>(npcId);
    if (!npc)
        return;
    if (!IsInRange(Ranges::Adjecent, npc))
        return;

    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item item;
    item.index = index;
    if (!client->Read(item))
    {
        LOG_ERROR << "Error reading item with index " << index << std::endl;
        return;
    }
    if (!npc->IsSellingItemType(item.type))
    {
        return;
    }
    if (count != 1 && AB::Entities::IsItemStackable(item.itemFlags))
    {
        LOG_ERROR << "Can not create more than one item from non stackable items" << std::endl;
        return;
    }
    const auto attribs = GetPossibleItemAttributes(item.type);
    if (attribs.find(static_cast<Attribute>(attributeIndex)) == attribs.end())
    {
        LOG_WARNING << "CHEAT: Player " << GetName() << " trying to craft an impossible item" << std::endl;
        return;
    }

    Components::InventoryComp& inv = *inventoryComp_;
    if (!inv.CheckInventoryCapacity(0, 1))
    {
        CallEvent<void(void)>(EVENT_ON_INVENTORYFULL);
        return;
    }

    auto* factory = GetSubsystem<ItemFactory>();
    // Stats contain the price
    const std::string maxStats = factory->GetMaxItemStats(item.uuid, npc->GetLevel(),
        static_cast<int>(attributeIndex));
    ItemStats stats;
    if (!stats.LoadFromString(maxStats))
    {
        LOG_ERROR << "Error loading stats from stream" << std::endl;
        return;
    }

    auto msg = Net::NetworkMessage::GetNew();
    auto notEnoughStuff = [&msg](uint32_t itemIndex)
    {
        Player::PlayerError((itemIndex == AB::Entities::MONEY_ITEM_INDEX) ?
            AB::GameProtocol::PlayerErrorValue::NotEnoughMoney :
            AB::GameProtocol::PlayerErrorValue::NoEnoughMaterials, *msg);
    };

    auto checkMats = [&](ItemStatIndex indexIndex, ItemStatIndex countIndex) -> bool
    {
        uint32_t _index = stats.GetValue(indexIndex, 0);
        uint32_t _count = stats.GetValue(countIndex, 0) * count;
        if (_index != 0 && _count != 0)
        {
            if (!inv.HaveInventoryItem(_index, _count))
            {
                notEnoughStuff(_index);
                return false;
            }
        }
        return true;
    };
    auto removeMats = [&](ItemStatIndex indexIndex, ItemStatIndex countIndex) -> bool
    {
        uint32_t _index = stats.GetValue(indexIndex, 0);
        uint32_t _count = stats.GetValue(countIndex, 0) * count;
        if (_index != 0 && _count != 0)
        {
            if (!inv.TakeInventoryItem(_index, _count, msg.get()))
                return false;
        }
        return true;
    };

    bool success = true;
    if (!checkMats(ItemStatIndex::Material1Index, ItemStatIndex::Material1Count))
        success = false;
    if (!checkMats(ItemStatIndex::Material2Index, ItemStatIndex::Material2Count))
        success = false;
    if (!checkMats(ItemStatIndex::Material3Index, ItemStatIndex::Material3Count))
        success = false;
    if (!checkMats(ItemStatIndex::Material4Index, ItemStatIndex::Material4Count))
        success = false;

    if (success)
    {
        uint32_t newItemId = factory->CreatePlayerItem(*this, item.uuid, AB::Entities::StoragePlace::Inventory, count, maxStats);
        if (newItemId == 0)
        {
            LOG_ERROR << "Error creating item " << item.uuid << std::endl;
            return;
        }
        inv.SetInventoryItem(newItemId, msg.get());

        success = removeMats(ItemStatIndex::Material1Index, ItemStatIndex::Material1Count);
        ASSERT(success);
        success = removeMats(ItemStatIndex::Material2Index, ItemStatIndex::Material2Count);
        ASSERT(success);
        success = removeMats(ItemStatIndex::Material3Index, ItemStatIndex::Material3Count);
        ASSERT(success);
        success = removeMats(ItemStatIndex::Material4Index, ItemStatIndex::Material4Count);
        ASSERT(success);
        (void)success;
    }

    WriteToOutput(*msg);
}

void Player::CRQSalvageItem(uint16_t kitPos, uint16_t pos)
{
    auto* item = inventoryComp_->GetInventoryItem(pos);
    if (!item)
        return;
    if (!item->IsSalvageable())
        return;

    auto* kit = inventoryComp_->GetInventoryItem(kitPos);
    if (!kit)
        return;
    if (kit->data_.index != AB::Entities::SALVAGE_KIT_ITEM_INDEX)
        return;

    const auto mat = item->GetSalvageMaterial();
    if (mat.first == 0 || mat.second == 0)
        return;

    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item eitem;
    eitem.index = mat.first;
    if (!client->Read(eitem))
    {
        LOG_ERROR << "Error reading item with index " << mat.first << std::endl;
        return;
    }

    if (!inventoryComp_->CheckInventoryCapacity(0, 1))
    {
        CallEvent<void(void)>(EVENT_ON_INVENTORYFULL);
        return;
    }

    auto msg = Net::NetworkMessage::GetNew();
    auto* factory = GetSubsystem<ItemFactory>();
    ItemTransaction statsTrans(*kit);
    if (!kit->Consume())
    {
        LOG_ERROR << "Error consuming" << std::endl;
        return;
    }

    uint32_t newItemId = factory->CreatePlayerItem(*this, eitem.uuid, AB::Entities::StoragePlace::Inventory, mat.second);
    if (newItemId == 0)
    {
        LOG_ERROR << "Error creating item " << eitem.uuid << std::endl;
        return;
    }

    if (!inventoryComp_->DestroyInventoryItem(pos))
    {
        LOG_ERROR << "Error destroying item at position " << static_cast<int>(pos) << std::endl;
        return;
    }
    msg->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
    AB::Packets::Server::InventoryItemDelete deletePacket = {
        pos
    };
    AB::Packets::Add(deletePacket, *msg);
    statsTrans.Commit();
    Components::InventoryComp::WriteItemUpdate(kit, msg.get());

    inventoryComp_->SetInventoryItem(newItemId, msg.get());
    WriteToOutput(*msg);
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
    friendList_ = ea::make_unique<FriendList>(data_.accountUuid);
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
    case AB::GameProtocol::CommandType::Clear:
    case AB::GameProtocol::CommandType::History:
    case AB::GameProtocol::CommandType::Quit:
    case AB::GameProtocol::CommandType::Time:
    case AB::GameProtocol::CommandType::ClientPrediction:
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
    if (account_.type < AB::Entities::AccountType::Gamemaster)
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
    ea::shared_ptr<Player> target = GetSubsystem<PlayerManager>()->GetPlayerByName(name);
    if (target)
    {
        // Found a player with the name so the target is on this server.
        ea::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Whisper, target->id_);
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
            ea::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Whisper, character.uuid);
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
    ea::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Guild, account_.guildUuid);
    if (channel)
        channel->Talk(*this, arguments);
}

void Player::HandleChatTradeCommand(const std::string& arguments, Net::NetworkMessage&)
{
    ea::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Trade, 0);
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
    message.AddByte(AB::GameProtocol::ServerPacketType::ObjectForcePosition);
    AB::Packets::Server::ObjectPositionUpdate packet = {
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
        static_cast<uint32_t>((sa::time::tick() - loginTime_) / 1000);
    // In seconds
    const uint32_t age = static_cast<uint32_t>((sa::time::tick() - data_.creation) / 1000);

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
    if (account_.type < AB::Entities::AccountType::Gamemaster)
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
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerMessage);
    std::stringstream ss;
    ss << std::to_string(deathStats_[AB::Entities::DeathStatIndexCount].GetInt()) << "|";
    ss << std::to_string(GetXp() - deathStats_[AB::Entities::DeathStatIndexAtXp].GetInt());
    AB::Packets::Server::ServerMessage packet = {
        static_cast<uint8_t>(AB::GameProtocol::ServerMessageType::Deaths),
        GetName(),
        ss.str()
    };
    AB::Packets::Add(packet, *nmsg);
    WriteToOutput(*nmsg);
}

void Player::HandleDieCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountType::God)
    {
        HandleUnknownCommand();
        return;
    }

    Die(nullptr);
}

void Player::HandleInstancesCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountType::God)
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
    ea::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Map, GetGame()->id_);
    if (channel)
        channel->Talk(*this, arguments);
}

void Player::HandlePartyChatCommand(const std::string& arguments, Net::NetworkMessage&)
{
    ea::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Party, GetParty()->GetId());
    if (channel)
        channel->Talk(*this, arguments);
}

void Player::HandleGodModeCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountType::Gamemaster)
    {
        HandleUnknownCommand();
        return;
    }

    static const uint32_t EFFECTINDEX_UNDESTROYABLE = 900000;

    if (effectsComp_->HasEffect(EFFECTINDEX_UNDESTROYABLE))
        effectsComp_->RemoveEffect(EFFECTINDEX_UNDESTROYABLE);
    else
        effectsComp_->AddEffect(ea::shared_ptr<Actor>(), EFFECTINDEX_UNDESTROYABLE, 0);
}

void Player::HandleGMInfoCommand(const std::string& message, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountType::Gamemaster)
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
    if (account_.type < AB::Entities::AccountType::God)
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
    if (account_.type < AB::Entities::AccountType::God)
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
    if (account_.type < AB::Entities::AccountType::Gamemaster)
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
        Math::Vector3 pos = player->transformation_.position_;
        // Random pos around target
        auto rng = GetSubsystem<Crypto::Random>();
        pos.x_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
        pos.z_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
        moveComp_->SetPosition(pos);
        moveComp_->forcePosition_ = true;
        return;
    }
    // Enter the same instance as the player
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
    ASSERT(GetParty());
    if (!GetParty()->IsLeader(*this))
        return;

    ASSERT(HasGame());
    auto game = GetGame();
    if (Utils::Uuid::IsEmpty(game->data_.queueMapUuid))
        return;

    auto* client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::QueueAdd;
    sa::PropWriteStream stream;
    stream.WriteString(data_.uuid);
    stream.WriteString(game->data_.queueMapUuid);
    msg.SetPropStream(stream);
    client->Write(msg);
    queueing_ = true;
}

void Player::CRQUnqueueForMatch()
{
    ASSERT(GetParty());
    if (!GetParty()->IsLeader(*this))
        return;

    auto* client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::QueueRemove;
    sa::PropWriteStream stream;
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
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::NpcHasQuest);
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
    {
        client_->ChangeInstance(mapUuid, instanceUuid);
    }
    LOG_ERROR << "client_ = null" << std::endl;
}

void Player::PingPosition(const Math::Vector3& worldPos)
{
    LOG_INFO << "pos " << worldPos << std::endl;
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerPacketType::PositionPinged);
    AB::Packets::Server::PositionPinged packet = {
        id_,
        { worldPos.x_, worldPos.y_, worldPos.z_ }
    };
    AB::Packets::Add(packet, *nmsg);
    GetParty()->WriteToMembers(*nmsg);
}

}
