#include "stdafx.h"
#include "Player.h"
#include "Logger.h"
#include "Chat.h"
#include "Random.h"
#include "MailBox.h"
#include "FriendList.h"
#include "PlayerManager.h"
#include "IOMail.h"
#include "StringUtils.h"
#include "Application.h"
#include <AB/Entities/Character.h>
#include "Profiler.h"
#include "GameManager.h"
#include "PartyManager.h"
#include "Item.h"
#include <AB/Entities/PlayerItemList.h>
#include <AB/Entities/AccountItemList.h>
#include "ItemDrop.h"
#include "ItemsCache.h"
#include "UuidUtils.h"
#include "IOAccount.h"
#include "Scheduler.h"
#include "IOGame.h"
#include "Guild.h"
#include "GuildManager.h"
#include "IOPlayer.h"

namespace Game {

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    Actor(),
    client_(client), mailBox_(nullptr),
    party_(nullptr),
    resigned_(false),
    loginTime_(0),
    logoutTime_(0),
    lastPing_(0),
    questComp_(std::make_unique<Components::QuestComp>(*this))
{
    events_.Subscribe<void(AB::GameProtocol::CommandTypes, const std::string&, Net::NetworkMessage&)>(EVENT_ON_HANDLECOMMAND,
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
        party_->gameId_ = game->id_;
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
    return GetGame()->data_.type != AB::Entities::GameTypeOutpost ||
        account_.type >= AB::Entities::AccountTypeGamemaster;
}

bool Player::CanUseSkill() const
{
    return GetGame()->data_.type != AB::Entities::GameTypeOutpost ||
        account_.type >= AB::Entities::AccountTypeGamemaster;
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
    Actor::AdvanceLevel();
    if (data_.level < LEVEL_CAP)
        ++data_.level;
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
    msg->AddByte(AB::GameProtocol::GamePong);
    // Depending on the timezone of server and client the server may also be behind, i.e. difference is negative.
    msg->Add<int32_t>(static_cast<int32_t>(clientTick - Utils::Tick()));
    WriteToOutput(*msg);
}

void Player::TriggerDialog(uint32_t dialogIndex)
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::DialogTrigger);
    msg->Add<uint32_t>(dialogIndex);
    WriteToOutput(*msg);
}

void Player::UpdateMailBox()
{
    if (!mailBox_ && !Utils::Uuid::IsEmpty(data_.accountUuid))
        mailBox_ = std::make_unique<MailBox>(data_.accountUuid);
    if (mailBox_)
        mailBox_->Update();
}

void Player::CRQGetMailHeaders()
{
    UpdateMailBox();
    if (!mailBox_)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::MailHeaders);
    msg->Add<uint16_t>(static_cast<uint16_t>(mailBox_->GetTotalMailCount()));
    const AB::Entities::MailList& mails = mailBox_->GetMails();
    for (const auto& mail : mails.mails)
    {
        msg->AddString(mail.uuid);
        msg->AddString(mail.fromName);
        msg->AddString(mail.subject);
        msg->Add<int64_t>(mail.created);
        msg->AddByte(mail.isRead ? 1 : 0);
    }
    WriteToOutput(*msg);
}

void Player::CRQGetInventory()
{
    const size_t count = inventoryComp_->GetInventoryCount();
    if (count == 0)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::InventoryContent);
    msg->Add<uint16_t>(static_cast<uint16_t>(count));
    inventoryComp_->VisitInventory([&msg](const Item& current)
    {
        msg->Add<uint16_t>(current.data_.type);
        msg->Add<uint32_t>(current.data_.index);
        msg->Add<uint8_t>(static_cast<uint8_t>(current.concreteItem_.storagePlace));
        msg->Add<uint16_t>(current.concreteItem_.storagePos);
        msg->Add<uint32_t>(current.concreteItem_.count);
        msg->Add<uint16_t>(current.concreteItem_.value);
        return Iteration::Continue;
    });

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
    msg->AddByte(AB::GameProtocol::InventoryItemDelete);
    msg->Add<uint16_t>(pos);
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
    msg->AddByte(AB::GameProtocol::InventoryItemDelete);
    msg->Add<uint16_t>(pos);
    // TODO: Write new equipment
//    msg->AddByte(AB::GameProtocol::EquipmentItemUpdate);
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
    msg->AddByte(AB::GameProtocol::InventoryItemDelete);
    msg->Add<uint16_t>(pos);

    // Add to chest
    inventoryComp_->SetChestItem(itemId, msg.get());
    WriteToOutput(*msg);
}

void Player::CRQDropInventoryItem(uint16_t pos)
{
    uint32_t itemId = inventoryComp_->RemoveInventoryItem(pos);
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (item)
    {

        item->concreteItem_.storagePlace = AB::Entities::StoragePlaceScene;
        item->concreteItem_.storagePos = 0;
        auto rng = GetSubsystem<Crypto::Random>();
        std::shared_ptr<ItemDrop> drop = std::make_shared<ItemDrop>(item->id_);
        drop->transformation_.position_ = transformation_.position_;
        // Random pos around dropper
        drop->transformation_.position_.y_ += 0.2f;
        drop->transformation_.position_.x_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
        drop->transformation_.position_.z_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
        drop->SetSource(GetThis());
        GetGame()->SpawnItemDrop(drop);

        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::InventoryItemDelete);
        msg->Add<uint16_t>(pos);
        WriteToOutput(*msg);
    }
}

void Player::CRQGetChest()
{
    const size_t count = inventoryComp_->GetChestCount();
    if (count == 0)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::ChestContent);
    msg->Add<uint16_t>(static_cast<uint16_t>(count));
    inventoryComp_->VisitChest([&msg](const Item& current)
    {
        msg->Add<uint16_t>(current.data_.type);
        msg->Add<uint32_t>(current.data_.index);
        msg->Add<uint8_t>(static_cast<uint8_t>(current.concreteItem_.storagePlace));
        msg->Add<uint16_t>(current.concreteItem_.storagePos);
        msg->Add<uint32_t>(current.concreteItem_.count);
        msg->Add<uint16_t>(current.concreteItem_.value);
        return Iteration::Continue;
    });

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
        msg->AddByte(AB::GameProtocol::ChestItemDelete);
        msg->Add<uint16_t>(pos);
        WriteToOutput(*msg);
    }
}

void Player::CRQSendMail(const std::string recipient, const std::string subject, const std::string body)
{
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    // Can not send mails to players I ignore
    if (!IsIgnored(recipient) && IO::IOMail_SendMailToPlayer(recipient, data_.accountUuid, GetName(), subject, body))
        nmsg->AddByte(AB::GameProtocol::ServerMessageTypeMailSent);
    else
        nmsg->AddByte(AB::GameProtocol::ServerMessageTypeMailNotSent);
    nmsg->AddString(recipient);
    nmsg->AddString("");                // Data
    WriteToOutput(*nmsg);
}

void Player::CRQGetMail(const std::string mailUuid)
{
    UpdateMailBox();
    if (!mailBox_)
        return;

    // mailUuid must not be a reference!
    AB::Entities::Mail m;
    if (mailBox_->ReadMail(mailUuid, m))
    {
        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::MailComplete);
        msg->AddString(m.fromAccountUuid);
        msg->AddString(m.fromName);
        msg->AddString(m.toName);
        msg->AddString(m.subject);
        msg->AddString(m.message);
        msg->Add<int64_t>(m.created);
        msg->AddByte(m.isRead ? 1 : 0);
        WriteToOutput(*msg);
    }
}

void Player::CRQDeleteMail(const std::string mailUuid)
{
    // mailUuid must not be a reference!
    UpdateMailBox();
    if (!mailBox_)
        return;

    if (mailUuid.compare("all") == 0)
    {
        auto msg = Net::NetworkMessage::GetNew();
        mailBox_->DeleteAll();
        msg->AddByte(AB::GameProtocol::ServerMessage);
        msg->AddByte(AB::GameProtocol::ServerMessageTypeMailDeleted);
        msg->AddString(GetName());
        msg->AddString(mailUuid);
        WriteToOutput(*msg);
        return;
    }

    AB::Entities::Mail m;
    if (mailBox_->DeleteMail(mailUuid, m))
    {
        auto msg = Net::NetworkMessage::GetNew();
        msg->AddByte(AB::GameProtocol::ServerMessage);
        msg->AddByte(AB::GameProtocol::ServerMessageTypeMailDeleted);
        msg->AddString(GetName());
        msg->AddString(mailUuid);
        WriteToOutput(*msg);
    }
}

void Player::NotifyNewMail()
{
    UpdateMailBox();
    if (!mailBox_)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    if (mailBox_->GetTotalMailCount() > 0)
    {
        // Notify player there are new emails since last check.
        msg->AddByte(AB::GameProtocol::ServerMessage);
        msg->AddByte(AB::GameProtocol::ServerMessageTypeNewMail);
        msg->AddString(GetName());
        msg->AddString(std::to_string(mailBox_->GetTotalMailCount()));
    }
    if (mailBox_->GetTotalMailCount() >= AB::Entities::Limits::MAX_MAIL_COUNT)
    {
        // Notify player that mailbox is full.
        msg->AddByte(AB::GameProtocol::ServerMessage);
        msg->AddByte(AB::GameProtocol::ServerMessageTypeMailboxFull);
        msg->AddString(GetName());
        msg->AddString(std::to_string(mailBox_->GetTotalMailCount()));
    }
    if (msg->GetSize() != 0)
        WriteToOutput(*msg);
}

void Player::CRQAddFriend(const std::string playerName, AB::Entities::FriendRelation relation)
{
    auto res = friendList_->AddFriendByName(playerName, relation);

    auto msg = Net::NetworkMessage::GetNew();
    switch (res)
    {
    case FriendList::Error::Success:
    {
        AB::Entities::Friend f;
        friendList_->GetFriendByName(playerName, f);
        msg->AddByte(AB::GameProtocol::FriendAdded);
        msg->AddString(f.friendUuid);
        msg->Add<uint8_t>(f.relation);
        break;
    }
    case FriendList::Error::NoFriend:
        // N/A
    case FriendList::Error::AlreadyFriend:
    case FriendList::Error::InternalError:
        // Do nothing
        break;
    case FriendList::Error::PlayerNotFound:
        msg->AddByte(AB::GameProtocol::ServerMessage);
        msg->AddByte(AB::GameProtocol::ServerMessageTypePlayerNotFound);
        msg->AddString(GetName());
        msg->AddString(playerName);
        break;
    }

    if (msg->GetSize() != 0)
        WriteToOutput(*msg);
}

void Player::CRQRemoveFriend(const std::string accountUuid)
{
    AB::Entities::Friend f;

    if (!friendList_->GetFriendByAccount(accountUuid, f))
        return;

    auto res = friendList_->Remove(accountUuid);

    auto msg = Net::NetworkMessage::GetNew();
    switch (res)
    {
    case FriendList::Error::Success:
        msg->AddByte(AB::GameProtocol::FriendRemoved);
        msg->AddString(accountUuid);
        msg->Add<uint8_t>(f.relation);
        break;
    case FriendList::Error::AlreadyFriend:
        // N/A
    case FriendList::Error::NoFriend:
    case FriendList::Error::InternalError:
    case FriendList::Error::PlayerNotFound:
        // Do nothing
        break;
    }

    if (msg->GetSize() != 0)
        WriteToOutput(*msg);
}

void Player::CRQChangeFriendNick(const std::string accountUuid, const std::string newName)
{
    AB::Entities::Friend f;

    if (!friendList_->GetFriendByAccount(accountUuid, f))
        return;

    auto res = friendList_->ChangeNickname(accountUuid, newName);

    auto msg = Net::NetworkMessage::GetNew();
    switch (res)
    {
    case FriendList::Error::Success:
        msg->AddByte(AB::GameProtocol::FriendRenamed);
        msg->AddString(accountUuid);
        msg->Add<uint8_t>(f.relation);
        msg->AddString(newName);
        break;
    case FriendList::Error::PlayerNotFound:
        // Do nothing
        msg->AddByte(AB::GameProtocol::ServerMessage);
        msg->AddByte(AB::GameProtocol::ServerMessageTypePlayerNotFound);
        msg->AddString(GetName());
        msg->AddString(accountUuid);
        break;
    case FriendList::Error::AlreadyFriend:
        // N/A
    case FriendList::Error::NoFriend:
    case FriendList::Error::InternalError:
        // Do nothing
        break;
    }

    if (msg->GetSize() != 0)
        WriteToOutput(*msg);
}

void Player::SendPlayerInfo(const AB::Entities::Character& ch, uint32_t fields)
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::PlayerInfo);
    msg->Add<uint32_t>(fields);
    msg->AddString(ch.accountUuid);
    AB::Entities::Friend f;
    bool isFriend = friendList_->GetFriendByAccount(ch.accountUuid, f);
    if (fields & AB::GameProtocol::PlayerInfoFieldName)
    {
        if (isFriend)
            msg->AddString(f.friendName);
        else
            msg->AddString(ch.name);
    }
    if (fields & AB::GameProtocol::PlayerInfoFieldRelation)
        msg->Add<uint8_t>(f.relation);

    AB::Entities::Account account;
    account.uuid = ch.accountUuid;
    AB::Entities::Character currentToon;
    IO::IOAccount_GetAccountInfo(account, currentToon);
    if (f.relation != AB::Entities::FriendRelationIgnore && account.onlineStatus != AB::Entities::OnlineStatusInvisible)
    {
        // If success == false -> offline, empty toon name
        if (fields & AB::GameProtocol::PlayerInfoFieldOnlineStatus)
            msg->Add<uint8_t>(account.onlineStatus);
        if (fields & AB::GameProtocol::PlayerInfoFieldCurrentName)
            msg->AddString(currentToon.name);
        if (fields & AB::GameProtocol::PlayerInfoFieldCurrentMap)
            msg->AddString(currentToon.currentMapUuid);
    }
    else
    {
        // Ignored always offline and no current toon
        if (fields & AB::GameProtocol::PlayerInfoFieldOnlineStatus)
            msg->Add<uint8_t>(AB::Entities::OnlineStatusOffline);
        if (fields & AB::GameProtocol::PlayerInfoFieldCurrentName)
            msg->AddString("");
        if (fields & AB::GameProtocol::PlayerInfoFieldCurrentMap)
            msg->AddString(Utils::Uuid::EMPTY_UUID);
    }
    // Guild info
    AB::Entities::GuildMember gm;
    IO::IOAccount_GetGuildMemberInfo(account, gm);
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildGuid)
        msg->AddString(account.guildUuid);
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildRole)
        msg->Add<uint8_t>(gm.role);
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildInviteName)
        msg->AddString(gm.inviteName);
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildInvited)
        msg->Add<int64_t>(gm.invited);
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildJoined)
        msg->Add<int64_t>(gm.joined);
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildExpires)
        msg->Add<int64_t>(gm.expires);

    WriteToOutput(*msg);
}

void Player::CRQGetPlayerInfoByAccount(const std::string accountUuid, uint32_t fields)
{
    AB::Entities::Character ch;
    bool found = IO::IOPlayer_GetPlayerInfoByAccount(accountUuid, ch);
    if (!found)
        // If there is no such thing, we just don't reply to this request
        return;
    SendPlayerInfo(ch, fields);
}

void Player::CRQGetPlayerInfoByName(const std::string name, uint32_t fields)
{
    AB::Entities::Character ch;
    bool found = IO::IOPlayer_GetPlayerInfoByName(name, ch);
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
    msg->AddByte(AB::GameProtocol::GuildMemberList);
    msg->Add<uint16_t>(static_cast<uint16_t>(members.members.size()));

    for (const AB::Entities::GuildMember& member : members.members)
    {
        msg->AddString(member.accountUuid);
    };
    WriteToOutput(*msg);
}

void Player::CRQGetGuildInfo()
{
    auto* gm = GetSubsystem<GuildManager>();
    auto guild = gm->Get(account_.guildUuid);
    if (!guild)
        return;

    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::GuildInfo);
    msg->AddString(guild->data_.uuid);
    msg->AddString(guild->data_.name);
    msg->AddString(guild->data_.tag);
    msg->Add<int64_t>(guild->data_.creation);
    msg->AddString(guild->data_.creatorAccountUuid);
    WriteToOutput(*msg);
}

void Player::CRQGetFriendList()
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::FriendList);
    msg->Add<uint16_t>(static_cast<uint16_t>(friendList_->Count()));
    friendList_->VisitAll([&msg](const AB::Entities::Friend& current)
    {
        msg->AddString(current.friendUuid);

        return Iteration::Continue;
    });
    WriteToOutput(*msg);
}

void Player::WriteToOutput(const Net::NetworkMessage& message)
{
    if (client_)
        client_->WriteToOutput(message);
    else
        LOG_ERROR << "client_ expired" << std::endl;
}

void Player::OnPingObject(uint32_t targetId, AB::GameProtocol::ObjectCallType type, int skillIndex)
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::GameObjectPingTarget);
    msg->Add<uint32_t>(id_);
    msg->Add<uint32_t>(targetId);
    msg->Add<uint8_t>(static_cast<uint8_t>(type));
    msg->Add<int8_t>(static_cast<int8_t>(skillIndex));
    GetParty()->WriteToMembers(*msg);
}

void Player::OnInventoryFull()
{
    auto msg = Net::NetworkMessage::GetNew();
    msg->AddByte(AB::GameProtocol::PlayerError);
    msg->AddByte(AB::GameProtocol::PlayerErrorInventoryFull);
    WriteToOutput(*msg);
}

void Player::SetParty(std::shared_ptr<Party> party)
{
    if (party_)
    {
        if (party && (party_->id_ == party->id_))
            return;
        party_->Remove(*this, false);
    }

    if (party)
    {
        party_ = party;
        data_.partyUuid = party->data_.uuid;
    }
    else
    {
        // Create new party
        data_.partyUuid.clear();
        party_ = GetSubsystem<PartyManager>()->GetByUuid(data_.partyUuid);
        party_->SetPartySize(GetGame()->data_.partySize);
        data_.partyUuid = party_->data_.uuid;
    }
    party_->Set(GetThis());
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
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
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
            nmsg->AddByte(AB::GameProtocol::PartyPlayerInvited);
            nmsg->Add<uint32_t>(id_);                             // Leader
            nmsg->Add<uint32_t>(playerId);                        // Invitee
            nmsg->Add<uint32_t>(party_->id_);
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
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
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
            if (!party_->Remove(*player))
                return;
            nmsg->AddByte(AB::GameProtocol::PartyPlayerRemoved);
            removedMember = true;
        }
        else if (party_->IsInvited(*player))
        {
            if (!party_->RemoveInvite(player))
                return;
            nmsg->AddByte(AB::GameProtocol::PartyInviteRemoved);
        }
        else
            return;

        nmsg->Add<uint32_t>(id_);                 // Leader
        nmsg->Add<uint32_t>(playerId);            // Member
        nmsg->Add<uint32_t>(party_->id_);
        party_->WriteToMembers(*nmsg);

        // Also send to player which is removed already
        player->WriteToOutput(*nmsg);
    }

    if (removedMember)
    {
        // The kicked player needs a new party
        player->SetParty(std::shared_ptr<Party>());
        auto nmsg = Net::NetworkMessage::GetNew();
        nmsg->AddByte(AB::GameProtocol::PartyPlayerAdded);
        nmsg->Add<uint32_t>(player->id_);                           // Acceptor
        nmsg->Add<uint32_t>(player->id_);                           // Leader
        nmsg->Add<uint32_t>(player->GetParty()->id_);
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
        nmsg->AddByte(AB::GameProtocol::PartyPlayerRemoved);
        auto leader = party_->GetLeader();
        nmsg->Add<uint32_t>(leader ? leader->id_ : 0);
        nmsg->Add<uint32_t>(id_);
        nmsg->Add<uint32_t>(party_->id_);
        party_->WriteToMembers(*nmsg);
        party_->Remove(*this);
    }

    {
        // We need a new party
        SetParty(std::shared_ptr<Party>());
        auto nmsg = Net::NetworkMessage::GetNew();
        nmsg->AddByte(AB::GameProtocol::PartyPlayerAdded);
        nmsg->Add<uint32_t>(id_);                           // Acceptor
        nmsg->Add<uint32_t>(id_);                           // Leader
        nmsg->Add<uint32_t>(party_->id_);
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
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;

    std::shared_ptr<Player> leader = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (leader)
    {
        // Leave current party
        PartyLeave();
        if (leader->GetParty()->Add(GetThis()))
        {
            auto nmsg = Net::NetworkMessage::GetNew();
            nmsg->AddByte(AB::GameProtocol::PartyPlayerAdded);
            nmsg->Add<uint32_t>(id_);                           // Acceptor
            nmsg->Add<uint32_t>(playerId);                      // Leader
            nmsg->Add<uint32_t>(party_->id_);
            party_->WriteToMembers(*nmsg);
#ifdef DEBUG_GAME
            LOG_DEBUG << "Acceptor: " << id_ << ", Leader: " << playerId << ", Party: " << party_->id_ << std::endl;
#endif
        }
        // else party maybe full
    }
}

void Player::CRQPartyRejectInvite(uint32_t inviterId)
{
    // We are the rejector
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;
    std::shared_ptr<Player> leader = GetSubsystem<PlayerManager>()->GetPlayerById(inviterId);
    if (leader)
    {
        if (leader->GetParty()->RemoveInvite(GetThis()))
        {
            auto nmsg = Net::NetworkMessage::GetNew();
            nmsg->AddByte(AB::GameProtocol::PartyInviteRemoved);
            nmsg->Add<uint32_t>(inviterId);                // Leader
            nmsg->Add<uint32_t>(id_);                      // We
            nmsg->Add<uint32_t>(leader->GetParty()->id_);
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
        nmsg->AddByte(AB::GameProtocol::PartyInfoMembers);
        nmsg->Add<uint32_t>(partyId);
        size_t count = party->GetMemberCount();
        nmsg->AddByte(static_cast<uint8_t>(count));
        // We also need invalid (i.e. not yet connected) members,
        // therefore we can not use Party::VisitMembers()
        const auto& members = party->GetMembers();
        for (auto& m : members)
        {
            if (auto sm = m.lock())
                nmsg->Add<uint32_t>(sm->id_);
            else
                nmsg->Add<uint32_t>(0);
        }
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

bool Player::IsIgnored(const Player& player)
{
    return friendList_->IsIgnored(player.account_.uuid);
}

bool Player::IsIgnored(const std::string& name)
{
    return friendList_->IsIgnoredByName(name);
}

bool Player::IsFriend(const Player& player)
{
    return friendList_->IsFriend(player.account_.uuid);
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

void Player::OnHandleCommand(AB::GameProtocol::CommandTypes type,
    const std::string& arguments, Net::NetworkMessage& message)
{
    switch (type)
    {
    case AB::GameProtocol::CommandTypeUnknown:
        break;
    case AB::GameProtocol::CommandTypeIp:
    case AB::GameProtocol::CommandTypePrefPath:
    case AB::GameProtocol::CommandTypeHelp:
    case AB::GameProtocol::CommandTypeQuit:
        // Client side only
        break;
    case AB::GameProtocol::CommandTypeChatGeneral:
        HandleGeneralChatCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeChatParty:
        HandlePartyChatCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeRoll:
        HandleRollCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeSit:
        HandleSitCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeStand:
        HandleStandCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeCry:
        HandleCryCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeTaunt:
        HandleTauntCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypePonder:
        HandlePonderCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeWave:
        HandleWaveCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeLaugh:
        HandleLaughCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeAge:
        HandleAgeCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeHealth:
        HandleHpCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeXp:
        HandleXpCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypePos:
        HandlePosCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeChatWhisper:
        HandleWhisperCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeChatGuild:
        HandleChatGuildCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeChatTrade:
        HandleChatTradeCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeResign:
        HandleResignCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeStuck:
        HandleStuckCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeServerId:
        HandleServerIdCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeDie:
        HandleDieCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeDeaths:
        HandleDeathsCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeInstances:
        HandleInstancesCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeGodMode:
        HandleGodModeCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeGMInfo:
        HandleGMInfoCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeEnterMap:
        HandleEnterMapCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeEnterInstance:
        HandleEnterInstanceCommand(arguments, message);
        break;
    case AB::GameProtocol::CommandTypeGotoPlayer:
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

    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    // Since it's more for debugging, it's only available for >= GM
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypeServerId);
    nmsg->AddString(GetName());
    nmsg->AddString(Application::Instance->GetServerId());
    WriteToOutput(*nmsg);
}

void Player::HandleWhisperCommand(const std::string& arguments, Net::NetworkMessage&)
{
    size_t p = arguments.find(',');
    if (p == std::string::npos)
        return;

    const std::string name = arguments.substr(0, p);
    const std::string msg = Utils::LeftTrim(arguments.substr(p + 1, std::string::npos));
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
                nmsg->AddByte(AB::GameProtocol::ServerMessage);
                nmsg->AddByte(AB::GameProtocol::ServerMessageTypePlayerGotMessage);
                nmsg->AddString(name);
                nmsg->AddString(msg);
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
        std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Whisper, character.uuid);
        if (channel->Talk(*this, msg))
        {
            auto nmsg = Net::NetworkMessage::GetNew();
            nmsg->AddByte(AB::GameProtocol::ServerMessage);
            nmsg->AddByte(AB::GameProtocol::ServerMessageTypePlayerGotMessage);
            nmsg->AddString(name);
            nmsg->AddString(msg);
            WriteToOutput(*nmsg);
            return;
        }
    }

    // Send not online message
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypePlayerNotOnline);
    nmsg->AddString(GetName());
    nmsg->AddString(name);
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
    if (GetGame()->data_.type <= AB::Entities::GameTypeOutpost)
        return;
    message.AddByte(AB::GameProtocol::ServerMessage);
    message.AddByte(AB::GameProtocol::ServerMessageTypePlayerResigned);
    message.AddString(GetName());
    message.AddString("");
    resigned_ = true;
}

void Player::HandleStuckCommand(const std::string&, Net::NetworkMessage& message)
{
    message.AddByte(AB::GameProtocol::GameObjectSetPosition);
    message.Add<uint32_t>(id_);
    message.Add<float>(transformation_.position_.x_);
    message.Add<float>(transformation_.position_.y_);
    message.Add<float>(transformation_.position_.z_);
}

void Player::HandleAgeCommand(const std::string&, Net::NetworkMessage&)
{
    // In seconds
    const uint32_t playTime = static_cast<uint32_t>(data_.onlineTime) +
        static_cast<uint32_t>((Utils::Tick() - loginTime_) / 1000);
    // In seconds
    const uint32_t age = static_cast<uint32_t>((Utils::Tick() - data_.creation) / 1000);

    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypeAge);
    nmsg->AddString(GetName());
    nmsg->AddString(std::to_string(age) + ":" + std::to_string(playTime));
    WriteToOutput(*nmsg);
}

void Player::HandleHpCommand(const std::string&, Net::NetworkMessage&)
{
    auto nmsg = Net::NetworkMessage::GetNew();
    const int maxHp = resourceComp_->GetMaxHealth();
    const int hp = resourceComp_->GetHealth();
    const int maxE = resourceComp_->GetMaxEnergy();
    const int e = resourceComp_->GetEnergy();
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypeHp);
    nmsg->AddString(GetName());
    nmsg->AddString(std::to_string(hp) + ":" + std::to_string(maxHp) + "|" + std::to_string(e) + ":" + std::to_string(maxE));
    WriteToOutput(*nmsg);
}

void Player::HandleXpCommand(const std::string&, Net::NetworkMessage&)
{
    auto nmsg = Net::NetworkMessage::GetNew();
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypeXp);
    nmsg->AddString(GetName());
    nmsg->AddString(std::to_string(data_.xp) + "|" + std::to_string(data_.skillPoints));
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
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypePos);
    nmsg->AddString(GetName());
    nmsg->AddString(ss.str());
    WriteToOutput(*nmsg);
}

void Player::HandleRollCommand(const std::string& arguments, Net::NetworkMessage& message)
{
    if (Utils::IsNumber(arguments))
    {
        const int max = std::stoi(arguments);
        if (max >= ROLL_MIN && max <= ROLL_MAX)
        {
            const int res = static_cast<int>(GetSubsystem<Crypto::Random>()->GetFloat() * static_cast<float>(max)) + 1;
            message.AddByte(AB::GameProtocol::ServerMessage);
            message.AddByte(AB::GameProtocol::ServerMessageTypeRoll);
            message.AddString(GetName());
            message.AddString(std::to_string(res) + ":" + std::to_string(max));
        }
    }
}

void Player::HandleSitCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteSit);
}

void Player::HandleStandCommand(const std::string&, Net::NetworkMessage&)
{
    if (stateComp_.GetState() == AB::GameProtocol::CreatureStateEmoteSit)
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
}

void Player::HandleCryCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteCry);
}

void Player::HandleTauntCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteTaunt);
}

void Player::HandlePonderCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmotePonder);
}

void Player::HandleWaveCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteWave);
}

void Player::HandleLaughCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_->IsAttackState())
        attackComp_->Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteLaugh);
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
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypeInstances);
    nmsg->AddString(GetName());
    auto* gameMan = GetSubsystem<GameManager>();
    const auto& games = gameMan->GetGames();
    std::stringstream ss;
    for (const auto& game : games)
    {
        ss << game.second->instanceData_.uuid << ",";
        ss << game.second->data_.uuid << ",";
        ss << game.second->data_.name << ";";
    }
    nmsg->AddString(ss.str());
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
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Party, GetParty()->id_);
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
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypeGMInfo);
    nmsg->AddString(GetName());
    nmsg->AddString(message);
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

    std::string uuid = IO::IOGame_GetGameUuidFromName(mapName);
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

    // May need to enter this commandd twice:
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
    nmsg->AddByte(AB::GameProtocol::ServerMessage);
    nmsg->AddByte(AB::GameProtocol::ServerMessageTypeUnknownCommand);
    nmsg->AddString(GetName());
    nmsg->AddString("");
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

    auto game = GetGame();
    if (game && game->data_.type != AB::Entities::GameTypeOutpost)
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
        client_->ChangeServerInstance(serverUuid, mapUuid, instanceUuid);
    else
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

void Player::ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid)
{
    resigned_ = false;
    if (client_)
        client_->ChangeInstance(mapUuid, instanceUuid);
    else
        LOG_ERROR << "client_ = null" << std::endl;
}

void Player::RegisterLua(kaguya::State& state)
{
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Actor>()
        .addFunction("GetParty", &Player::_LuaGetParty)
        .addFunction("ChangeMap", &Player::ChangeMap)
        .addFunction("TriggerDialog", &Player::TriggerDialog)
    );
}

}
