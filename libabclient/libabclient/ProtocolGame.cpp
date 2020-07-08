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


#include "ProtocolGame.h"
#include "TimeUtils.h"
#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>
#include <set>
#include <AB/Packets/ClientPackets.h>

namespace Client {

ProtocolGame::ProtocolGame(Receiver& receiver, Crypto::DHKeys& keys, asio::io_service& ioService) :
    Protocol(keys, ioService),
    receiver_(receiver),
    updateTick_(0),
    loggingOut_(false)
{
    checksumEnabled_ = ProtocolGame::UseChecksum;
    compressionEnabled_ = ENABLE_GAME_COMPRESSION;
    encryptEnabled_ = ENABLE_GAME_ENCRYTION;
    SetEncKey(AB::ENC_KEY);

    using namespace AB::GameProtocol;
    AddHandler<AB::Packets::Server::ServerJoined, ServerPacketType::ServerJoined>();
    AddHandler<AB::Packets::Server::ServerLeft, ServerPacketType::ServerLeft>();
    AddHandler<AB::Packets::Server::ChangeInstance, ServerPacketType::ChangeInstance>();
    AddHandler<AB::Packets::Server::EnterWorld, ServerPacketType::EnterWorld>();
    AddHandler<AB::Packets::Server::PlayerError, ServerPacketType::PlayerError>();
    AddHandler<AB::Packets::Server::PlayerAutorun, ServerPacketType::PlayerAutorun>();
    AddHandler<AB::Packets::Server::MailHeaders, ServerPacketType::MailHeaders>();
    AddHandler<AB::Packets::Server::MailComplete, ServerPacketType::MailComplete>();
    AddHandler<AB::Packets::Server::InventoryContent, ServerPacketType::InventoryContent>();
    AddHandler<AB::Packets::Server::InventoryItemUpdate, ServerPacketType::InventoryItemUpdate>();
    AddHandler<AB::Packets::Server::InventoryItemDelete, ServerPacketType::InventoryItemDelete>();
    AddHandler<AB::Packets::Server::ChestContent, ServerPacketType::ChestContent>();
    AddHandler<AB::Packets::Server::ChestItemUpdate, ServerPacketType::ChestItemUpdate>();
    AddHandler<AB::Packets::Server::ChestItemDelete, ServerPacketType::ChestItemDelete>();
    AddHandler<AB::Packets::Server::ObjectSpawnExisting, ServerPacketType::ObjectSpawnExisting>();
    AddHandler<AB::Packets::Server::ObjectSpawn, ServerPacketType::ObjectSpawn>();
    AddHandler<AB::Packets::Server::ObjectDespawn, ServerPacketType::ObjectDespawn>();
    AddHandler<AB::Packets::Server::ObjectPositionUpdate, ServerPacketType::ObjectPositionUpdate>();
    AddHandler<AB::Packets::Server::ObjectRotationUpdate, ServerPacketType::ObjectRotationUpdate>();
    AddHandler<AB::Packets::Server::ObjectStateChanged, ServerPacketType::ObjectStateChanged>();
    AddHandler<AB::Packets::Server::ObjectSpeedChanged, ServerPacketType::ObjectSpeedChanged>();
    AddHandler<AB::Packets::Server::ObjectTargetSelected, ServerPacketType::ObjectTargetSelected>();
    AddHandler<AB::Packets::Server::ObjectSkillFailure, ServerPacketType::ObjectSkillFailure>();
    AddHandler<AB::Packets::Server::ObjectUseSkill, ServerPacketType::ObjectUseSkill>();
    AddHandler<AB::Packets::Server::ObjectSkillSuccess, ServerPacketType::ObjectSkillSuccess>();
    AddHandler<AB::Packets::Server::ObjectAttackFailure, ServerPacketType::ObjectAttackFailure>();
    AddHandler<AB::Packets::Server::ObjectPingTarget, ServerPacketType::ObjectPingTarget>();
    AddHandler<AB::Packets::Server::ObjectEffectAdded, ServerPacketType::ObjectEffectAdded>();
    AddHandler<AB::Packets::Server::ObjectEffectRemoved, ServerPacketType::ObjectEffectRemoved>();
    AddHandler<AB::Packets::Server::ObjectDamaged, ServerPacketType::ObjectDamaged>();
    AddHandler<AB::Packets::Server::ObjectHealed, ServerPacketType::ObjectHealed>();
    AddHandler<AB::Packets::Server::ObjectProgress, ServerPacketType::ObjectProgress>();
    AddHandler<AB::Packets::Server::ObjectDroppedItem, ServerPacketType::ObjectDroppedItem>();
    AddHandler<AB::Packets::Server::ObjectForcePosition, ServerPacketType::ObjectForcePosition>();
    AddHandler<AB::Packets::Server::ObjectGroupMaskChanged, ServerPacketType::ObjectGroupMaskChanged>();
    AddHandler<AB::Packets::Server::ObjectSetAttackSpeed, ServerPacketType::ObjectSetAttackSpeed>();
    AddHandler<AB::Packets::Server::ServerMessage, ServerPacketType::ServerMessage>();
    AddHandler<AB::Packets::Server::ChatMessage, ServerPacketType::ChatMessage>();
    AddHandler<AB::Packets::Server::PartyPlayerInvited, ServerPacketType::PartyPlayerInvited>();
    AddHandler<AB::Packets::Server::PartyPlayerRemoved, ServerPacketType::PartyPlayerRemoved>();
    AddHandler<AB::Packets::Server::PartyPlayerAdded, ServerPacketType::PartyPlayerAdded>();
    AddHandler<AB::Packets::Server::PartyInviteRemoved, ServerPacketType::PartyInviteRemoved>();
    AddHandler<AB::Packets::Server::PartyResigned, ServerPacketType::PartyResigned>();
    AddHandler<AB::Packets::Server::PartyDefeated, ServerPacketType::PartyDefeated>();
    AddHandler<AB::Packets::Server::PartyMembersInfo, ServerPacketType::PartyMembersInfo>();
    AddHandler<AB::Packets::Server::ObjectResourceChanged, ServerPacketType::ObjectResourceChanged>();
    AddHandler<AB::Packets::Server::DialogTrigger, ServerPacketType::DialogTrigger>();
    AddHandler<AB::Packets::Server::FriendList, ServerPacketType::FriendList>();
    AddHandler<AB::Packets::Server::FriendAdded, ServerPacketType::FriendAdded>();
    AddHandler<AB::Packets::Server::FriendRemoved, ServerPacketType::FriendRemoved>();
    AddHandler<AB::Packets::Server::FriendRenamed, ServerPacketType::FriendRenamed>();
    AddHandler<AB::Packets::Server::GuildInfo, ServerPacketType::GuildInfo>();
    AddHandler<AB::Packets::Server::GuildMemberList, ServerPacketType::GuildMemberList>();
    AddHandler<AB::Packets::Server::QuestSelectionDialogTrigger, ServerPacketType::QuestSelectionDialogTrigger>();
    AddHandler<AB::Packets::Server::QuestDialogTrigger, ServerPacketType::QuestDialogTrigger>();
    AddHandler<AB::Packets::Server::NpcHasQuest, ServerPacketType::NpcHasQuest>();
    AddHandler<AB::Packets::Server::QuestDeleted, ServerPacketType::QuestDeleted>();
    AddHandler<AB::Packets::Server::QuestRewarded, ServerPacketType::QuestRewarded>();
    AddHandler<AB::Packets::Server::PlayerInfo, ServerPacketType::PlayerInfo>();
    AddHandler<AB::Packets::Server::SetObjectAttributeValue, ServerPacketType::SetObjectAttributeValue>();
    AddHandler<AB::Packets::Server::ObjectSecProfessionChanged, ServerPacketType::ObjectSecProfessionChanged>();
    AddHandler<AB::Packets::Server::ObjectSetSkill, ServerPacketType::ObjectSetSkill>();
    AddHandler<AB::Packets::Server::SkillTemplateLoaded, ServerPacketType::SkillTemplateLoaded>();
    AddHandler<AB::Packets::Server::TradeDialogTrigger, ServerPacketType::TradeDialogTrigger>();
    AddHandler<AB::Packets::Server::TradeCancel, ServerPacketType::TradeCancel>();
    AddHandler<AB::Packets::Server::TradeOffer, ServerPacketType::TradeOffer>();
    AddHandler<AB::Packets::Server::TradeAccepted, ServerPacketType::TradeAccepted>();
    AddHandler<AB::Packets::Server::MerchantItems, ServerPacketType::MerchantItems>();
    AddHandler<AB::Packets::Server::ItemPrice, ServerPacketType::ItemPrice>();
    AddHandler<AB::Packets::Server::CraftsmanItems, ServerPacketType::CraftsmanItems>();
}

void ProtocolGame::Login(const std::string& accountUuid,
    const std::string& authToken, const std::string& charUuid,
    const std::string& mapUuid,
    const std::string& instanceUuid,
    const std::string& host, uint16_t port)
{
    loggingOut_ = false;
    Connect(host, port, [=]()
    {
        firstRevc_ = true;

        // Login packet uses the default key
        SetEncKey(AB::ENC_KEY);

        OutputMessage msg;
        msg.Add<uint8_t>(ProtocolGame::ProtocolIdentifier);

        AB::Packets::Client::GameLogin packet;
        packet.clientOs = AB::CLIENT_OS_CURRENT;
        packet.protocolVersion = AB::PROTOCOL_VERSION;
        const DH_KEY& key = keys_.GetPublickKey();
        for (int i = 0; i < DH_KEY_LENGTH; ++i)
            packet.key[i] = key[i];
        packet.accountUuid = accountUuid;
        packet.authToken = authToken;
        packet.charUuid = charUuid;
        packet.mapUuid = mapUuid;
        packet.instanceUuid = instanceUuid;
        AB::Packets::Add(packet, msg);
        Send(msg);

        Receive();
    });
}

void ProtocolGame::OnReceive(InputMessage& message)
{
    try
    {
        ParseMessage(message);
        Receive();
    }
    catch (const std::exception&)
    {
        ProtocolError(AB::ErrorCodes::ErrorException);
    }
    if (firstRevc_)
    {
        firstRevc_ = false;
        // Switch now to the shared key
        keys_.GetSharedKey(serverKey_, encKey_);
    }
}

void ProtocolGame::OnError(ConnectionError connectionError, const asio::error_code& err)
{
    Protocol::OnError(connectionError, err);
    // Error 2 = End of file
    if (loggingOut_ && err.default_error_condition().value() == 2)
    {
        // When logging out a disconnect is expected
        loggingOut_ = false;
        return;
    }

    receiver_.OnNetworkError(connectionError, err);
}

void ProtocolGame::ParseMessage(InputMessage& message)
{
    using namespace AB::GameProtocol;

    ServerPacketType opCode = ServerPacketType::NoError;
    // One such message contains a variable number of packets and then some padding bytes.
    while (!message.Eof())
    {
        ServerPacketType prevCode = opCode;
        opCode = static_cast<ServerPacketType>(message.Get<uint8_t>());

#if 0
        if (opCode != ServerPacketType::GameUpdate && opCode != ServerPacketType::__Last)
        {
            std::stringstream ss3;
            ss3 << "ProtocolGame::ParseMessage(): Code " << static_cast<int>(opCode) << " " << GetServerPacketTypeName(opCode) <<
                " last code " << static_cast<int>(prevCode) << " " << GetServerPacketTypeName(prevCode) <<
                " unread size " << message.GetUnreadSize();
            LogMessage(ss3.str());
        }
#endif

        if (packetHandlers_.Exists(opCode))
        {
            packetHandlers_.Call(opCode, message);
            continue;
        }

        // There are some special codes we handle separately
        switch (opCode)
        {
        case ServerPacketType::KeyExchange:
            ParseKeyExchange(message);
            break;
        case ServerPacketType::ProtocolError:
        {
            auto packet = AB::Packets::Get<AB::Packets::Server::ProtocolError>(message);
            if (packet.code != 0)
                ProtocolError(static_cast<AB::ErrorCodes>(packet.code));
            break;
        }
        case ServerPacketType::GameStart:
            // This is not interesting for the client
            AB::Packets::Get<AB::Packets::Server::GameStart>(message);
            break;
        case ServerPacketType::GamePong:
        {
            auto packet = AB::Packets::Get<AB::Packets::Server::GamePong>(message);
            // Clock difference between client and server
            clockDiff_ = static_cast<int64_t>(packet.clockDiff);
            // Round trip time
            lastPing_ = static_cast<int>(AbTick() - pingTick_);
            receiver_.OnPong(lastPing_);
            break;
        }
        case ServerPacketType::GameUpdate:
        {
            auto packet = AB::Packets::Get<AB::Packets::Server::GameUpdate>(message);
            updateTick_ = packet.tick;
            break;
        }
        case ServerPacketType::__Last:
            // Padding bytes, i.e. end of message
            return;
        default:
        {
            std::stringstream ss2;
            ss2 << "ProtocolGame::ParseMessage(): Unknown packet code " << static_cast<int>(opCode) << " " << GetServerPacketTypeName(opCode) <<
                " last code " << static_cast<int>(prevCode) << " " << GetServerPacketTypeName(prevCode) <<
                " unread size " << message.GetUnreadSize();
            LogMessage(ss2.str());
            // Unknown packet, discard whole message
            return;
        }
        }
    }
}

void ProtocolGame::ParseKeyExchange(InputMessage& message)
{
    for (int i = 0; i < DH_KEY_LENGTH; ++i)
        serverKey_[i] = message.Get<uint8_t>();
}

void ProtocolGame::LogMessage(const std::string& message)
{
    receiver_.OnLog(message);
}

void ProtocolGame::Logout()
{
    loggingOut_ = true;
    AB::Packets::Client::Logout packet;
    SendPacket(AB::GameProtocol::ClientPacketTypes::Logout, packet);
}

void ProtocolGame::Ping()
{
    pingTick_ = AbTick();
    AB::Packets::Client::Ping packet = {
        pingTick_
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Ping, packet);
}

void ProtocolGame::ChangeMap(const std::string& mapUuid)
{
    AB::Packets::Client::ChangeMap packet = {
        mapUuid
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::ChangeMap, packet);
}

void ProtocolGame::GetMailHeaders()
{
    AB::Packets::Client::GetMailHeaders packet;
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetMailHeaders, packet);
}

void ProtocolGame::GetInventory()
{
    AB::Packets::Client::GetInventory packet;
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetInventory, packet);
}

void ProtocolGame::InventoryDestroyItem(uint16_t pos)
{
    AB::Packets::Client::InventoryDestroyItem packet = {
        pos
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::InventoryDestroyItem, packet);
}

void ProtocolGame::InventoryDropItem(uint16_t pos, uint32_t count)
{
    AB::Packets::Client::InventoryDropItem packet{
        pos,
        count
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::InventoryDropItem, packet);
}

void ProtocolGame::SetItemPos(AB::Entities::StoragePlace currentPlace, uint16_t currentPos,
    AB::Entities::StoragePlace place, uint16_t newPos, uint32_t count)
{
    AB::Packets::Client::SetItemPos packet{
        static_cast<uint8_t>(currentPlace), currentPos,
        static_cast<uint8_t>(place), newPos,
        count};
    SendPacket(AB::GameProtocol::ClientPacketTypes::SetItemPos, packet);
}

void ProtocolGame::GetChest()
{
    AB::Packets::Client::GetChest packet;
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetChest, packet);
}

void ProtocolGame::ChestDestroyItem(uint16_t pos)
{
    AB::Packets::Client::ChestDestroyItem packet = {
        pos
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::ChestDestroyItem, packet);
}

void ProtocolGame::DepositMoney(uint32_t amount)
{
    AB::Packets::Client::DespositWithdrawMoney packet{ amount };
    SendPacket(AB::GameProtocol::ClientPacketTypes::DepositMoney, packet);
}

void ProtocolGame::WithdrawMoney(uint32_t amount)
{
    AB::Packets::Client::DespositWithdrawMoney packet{ amount };
    SendPacket(AB::GameProtocol::ClientPacketTypes::WithdrawMoney, packet);
}

void ProtocolGame::SellItem(uint32_t npcId, uint16_t pos, uint32_t count)
{
    AB::Packets::Client::SellItem packet = {
        npcId, pos, count
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::SellItem, packet);
}

void ProtocolGame::BuyItem(uint32_t npcId, uint32_t id, uint32_t count)
{
    AB::Packets::Client::BuyItem packet = {
        npcId, id, count
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::BuyItem, packet);
}

void ProtocolGame::GetMerchantItems(uint32_t npcId, uint16_t itemType, const std::string& searchName, uint32_t page)
{
    AB::Packets::Client::GetMerchantItems packet = {
        npcId, itemType, searchName, static_cast<uint8_t>(page)
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetMerchantItems, packet);
}

void ProtocolGame::GetCraftsmanItems(uint32_t npcId, uint16_t itemType, const std::string& searchName, uint32_t page)
{
    AB::Packets::Client::GetCraftsmanItems packet = {
        npcId, itemType, searchName, static_cast<uint8_t>(page)
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetCraftsmanItems, packet);
}

void ProtocolGame::GetMail(const std::string& mailUuid)
{
    AB::Packets::Client::GetMail packet = {
        mailUuid
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetMail, packet);
}

void ProtocolGame::DeleteMail(const std::string& mailUuid)
{
    AB::Packets::Client::DeleteMail packet = {
        mailUuid
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::DeleteMail, packet);
}

void ProtocolGame::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    AB::Packets::Client::SendMail packet = {
        recipient,
        subject,
        body
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::SendMail, packet);
}

void ProtocolGame::GetPlayerInfoByName(const std::string& name, uint32_t fields)
{
    AB::Packets::Client::GetPlayerInfoByName packet = {
        name,
        fields
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetPlayerInfoByName, packet);
}

void ProtocolGame::GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields)
{
    AB::Packets::Client::GetPlayerInfoByAccount packet = {
        accountUuid,
        fields
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetPlayerInfoByAccount, packet);
}

void ProtocolGame::Move(uint8_t direction)
{
    AB::Packets::Client::Move packet = {
        direction
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Move, packet);
}

void ProtocolGame::Turn(uint8_t direction)
{
    AB::Packets::Client::Turn packet = {
        direction
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Turn, packet);
}

void ProtocolGame::SetDirection(float rad)
{
    AB::Packets::Client::SetDirection packet = {
        rad
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::SetDirection, packet);
}

void ProtocolGame::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    AB::Packets::Client::ClickObject packet = {
        sourceId,
        targetId
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::ClickObject, packet);
}

void ProtocolGame::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    AB::Packets::Client::SelectObject packet = {
        sourceId,
        targetId
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Select, packet);
}

void ProtocolGame::Command(AB::GameProtocol::CommandType type, const std::string& data)
{
    AB::Packets::Client::Command packet = {
        static_cast<uint8_t>(type),
        data
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Command, packet);
}

void ProtocolGame::GotoPos(const Vec3& pos)
{
    AB::Packets::Client::GotoPos packet = {
        { pos.x, pos.y, pos.z }
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Goto, packet);
}

void ProtocolGame::Follow(uint32_t targetId, bool ping)
{
    AB::Packets::Client::Follow packet = {
        targetId,
        ping
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Follow, packet);
}

void ProtocolGame::UseSkill(uint32_t index, bool ping)
{
    AB::Packets::Client::UseSkill packet = {
        static_cast<uint8_t>(index),
        ping
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::UseSkill, packet);
}

void ProtocolGame::Attack(bool ping)
{
    AB::Packets::Client::Attack packet = {
        ping
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Attack, packet);
}

void ProtocolGame::Cancel()
{
    AB::Packets::Client::Cancel packet = { };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Cancel, packet);
}

void ProtocolGame::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    AB::Packets::Client::SetPlayerState packet = {
        static_cast<uint8_t>(newState)
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::SetState, packet);
}

void ProtocolGame::PartyInvitePlayer(uint32_t targetId)
{
    AB::Packets::Client::PartyInvitePlayer packet = {
        targetId
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::PartyInvitePlayer, packet);
}

void ProtocolGame::PartyKickPlayer(uint32_t targetId)
{
    AB::Packets::Client::PartyKickPlayer packet = {
        targetId
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::PartyKickPlayer, packet);
}

void ProtocolGame::PartyLeave()
{
    AB::Packets::Client::PartyLeave packet = { };
    SendPacket(AB::GameProtocol::ClientPacketTypes::PartyLeave, packet);
}

void ProtocolGame::PartyAcceptInvite(uint32_t inviterId)
{
    AB::Packets::Client::PartyAcceptInvite packet = {
        inviterId
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::PartyAcceptInvite, packet);
}

void ProtocolGame::PartyRejectInvite(uint32_t inviterId)
{
    AB::Packets::Client::PartyRejectInvite packet = {
        inviterId
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::PartyRejectInvite, packet);
}

void ProtocolGame::PartyGetMembers(uint32_t partyId)
{
    AB::Packets::Client::PartyGetMembers packet = {
        partyId
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetPartyMembers, packet);
}

void ProtocolGame::QueueMatch()
{
    AB::Packets::Client::QueueMatch packet = { };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Queue, packet);
}

void ProtocolGame::UnqueueMatch()
{
    AB::Packets::Client::UnqueueMatch packet = { };
    SendPacket(AB::GameProtocol::ClientPacketTypes::Unqueue, packet);
}

void ProtocolGame::AddFriend(const std::string& name, AB::Entities::FriendRelation relation)
{
    AB::Packets::Client::AddFriend packet = {
        name,
        relation
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::AddFriend, packet);
}

void ProtocolGame::RemoveFriend(const std::string& accountUuid)
{
    AB::Packets::Client::RemoveFriend packet = {
        accountUuid
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::RemoveFriend, packet);
}

void ProtocolGame::RenameFriend(const std::string& accountUuid, const std::string& newName)
{
    AB::Packets::Client::RenameFriend packet{
        accountUuid,
        newName
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::RenameFriend, packet);
}

void ProtocolGame::UpdateFriendList()
{
    AB::Packets::Client::UpdateFriendList packet = { };
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetFriendList, packet);
}

void ProtocolGame::SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status)
{
    AB::Packets::Client::SetOnlineStatus packet = {
        status
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::SetOnlineStatus, packet);
}

void ProtocolGame::SetSecondaryProfession(uint32_t profIndex)
{
    AB::Packets::Client::SetSecondaryProfession packet = {
        profIndex
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::SetSecondaryProfession, packet);
}

void ProtocolGame::SetAttributeValue(uint32_t attribIndex, uint8_t value)
{
    AB::Packets::Client::SetAttributeValue packet = {
        attribIndex, value
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::SetAttributeValue, packet);
}

void ProtocolGame::EquipSkill(uint32_t skillIndex, uint8_t pos)
{
    AB::Packets::Client::EquipSkill packet = {
        skillIndex, pos
    };
    SendPacket(AB::GameProtocol::ClientPacketTypes::EquipSkill, packet);
}

void ProtocolGame::LoadSkillTemplate(const std::string& templ)
{
    AB::Packets::Client::LoadSkillTemplate packet;
    packet.templ = templ;
    SendPacket(AB::GameProtocol::ClientPacketTypes::LoadSkillTemplate, packet);
}

void ProtocolGame::TradeRequest(uint32_t targetId)
{
    AB::Packets::Client::TradeRequest packet{ targetId };
    SendPacket(AB::GameProtocol::ClientPacketTypes::TradeRequest, packet);
}

void ProtocolGame::TradeCancel()
{
    AB::Packets::Client::TradeCancel packet = { };
    SendPacket(AB::GameProtocol::ClientPacketTypes::TradeCancel, packet);
}

void ProtocolGame::TradeOffer(uint32_t money, std::vector<std::pair<uint16_t, uint32_t>>&& items)
{
    uint8_t count = static_cast<uint8_t>(items.size());
    AB::Packets::Client::TradeOffer packet{ money, count, std::move(items) };
    SendPacket(AB::GameProtocol::ClientPacketTypes::TradeOffer, packet);
}

void ProtocolGame::TradeAccept()
{
    AB::Packets::Client::TradeAccept packet = { };
    SendPacket(AB::GameProtocol::ClientPacketTypes::TradeAccept, packet);
}

void ProtocolGame::GetItemPrice(const std::vector<uint16_t>& items)
{
    AB::Packets::Client::GetItemPrice packet;
    packet.count = static_cast<uint8_t>(items.size());
    packet.items = items;
    SendPacket(AB::GameProtocol::ClientPacketTypes::GetItemsPrice, packet);
}

}
