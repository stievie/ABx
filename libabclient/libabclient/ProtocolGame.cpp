#include "stdafx.h"
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
    AddHandler<AB::Packets::Server::ServerJoined, ServerJoined>();
    AddHandler<AB::Packets::Server::ServerLeft, ServerLeft>();
    AddHandler<AB::Packets::Server::ChangeInstance, ChangeInstance>();
    AddHandler<AB::Packets::Server::EnterWorld, GameEnter>();
    AddHandler<AB::Packets::Server::GameError, PlayerError>();
    AddHandler<AB::Packets::Server::PlayerAutorun, PlayerAutoRun>();
    AddHandler<AB::Packets::Server::MailHeaders, MailHeaders>();
    AddHandler<AB::Packets::Server::MailComplete, MailComplete>();
    AddHandler<AB::Packets::Server::InventoryContent, InventoryContent>();
    AddHandler<AB::Packets::Server::InventoryItemUpdate, InventoryItemUpdate>();
    AddHandler<AB::Packets::Server::InventoryItemDelete, InventoryItemDelete>();
    AddHandler<AB::Packets::Server::ChestContent, ChestContent>();
    AddHandler<AB::Packets::Server::ChestItemUpdate, ChestItemUpdate>();
    AddHandler<AB::Packets::Server::ChestItemDelete, ChestItemDelete>();
    AddHandler<AB::Packets::Server::ObjectSpawnExisting, GameSpawnObjectExisting>();
    AddHandler<AB::Packets::Server::ObjectSpawn, GameSpawnObject>();
    AddHandler<AB::Packets::Server::ObjectDespawn, GameLeaveObject>();
    AddHandler<AB::Packets::Server::ObjectPosUpdate, GameObjectPositionChange>();
    AddHandler<AB::Packets::Server::ObjectRotationUpdate, GameObjectRotationChange>();
    AddHandler<AB::Packets::Server::ObjectStateChanged, GameObjectStateChange>();
    AddHandler<AB::Packets::Server::ObjectTargetSelected, GameObjectSelectTarget>();
    AddHandler<AB::Packets::Server::ObjectSkillFailure, GameObjectSkillFailure>();
    AddHandler<AB::Packets::Server::ObjectUseSkill, GameObjectUseSkill>();
    AddHandler<AB::Packets::Server::ObjectSkillSuccess, GameObjectEndUseSkill>();
    AddHandler<AB::Packets::Server::ObjectAttackFailure, GameObjectAttackFailure>();
    AddHandler<AB::Packets::Server::ObjectPingTarget, GameObjectPingTarget>();
    AddHandler<AB::Packets::Server::ObjectEffectAdded, GameObjectEffectAdded>();
    AddHandler<AB::Packets::Server::ObjectEffectRemoved, GameObjectEffectRemoved>();
    AddHandler<AB::Packets::Server::ObjectDamaged, GameObjectDamaged>();
    AddHandler<AB::Packets::Server::ObjectHealed, GameObjectHealed>();
    AddHandler<AB::Packets::Server::ObjectProgress, GameObjectProgress>();
    AddHandler<AB::Packets::Server::ObjectDroppedItem, GameObjectDropItem>();
    AddHandler<AB::Packets::Server::ObjectSetPosition, GameObjectSetPosition>();
    AddHandler<AB::Packets::Server::ServerMessage, ServerMessage>();
    AddHandler<AB::Packets::Server::ChatMessage, ChatMessage>();
    AddHandler<AB::Packets::Server::PartyPlayerInvited, PartyPlayerInvited>();
    AddHandler<AB::Packets::Server::PartyPlayerRemoved, PartyPlayerRemoved>();
    AddHandler<AB::Packets::Server::PartyPlayerAdded, PartyPlayerAdded>();
    AddHandler<AB::Packets::Server::PartyInviteRemoved, PartyInviteRemoved>();
    AddHandler<AB::Packets::Server::PartyResigned, PartyResigned>();
    AddHandler<AB::Packets::Server::PartyDefeated, PartyDefeated>();
    AddHandler<AB::Packets::Server::PartyMembersInfo, PartyInfoMembers>();
    AddHandler<AB::Packets::Server::ObjectResourceChanged, GameObjectResourceChange>();
    AddHandler<AB::Packets::Server::DialogTrigger, DialogTrigger>();
    AddHandler<AB::Packets::Server::FriendList, FriendList>();
    AddHandler<AB::Packets::Server::FriendAdded, FriendAdded>();
    AddHandler<AB::Packets::Server::FriendRemoved, FriendRemoved>();
    AddHandler<AB::Packets::Server::GuildInfo, GuildInfo>();
    AddHandler<AB::Packets::Server::GuildMemberList, GuildMemberList>();
    AddHandler<AB::Packets::Server::QuestSelectionDialogTrigger, QuestSelectionDialogTrigger>();
    AddHandler<AB::Packets::Server::QuestDialogTrigger, QuestDialogTrigger>();
    AddHandler<AB::Packets::Server::NpcHasQuest, QuestNpcHasQuest>();
    AddHandler<AB::Packets::Server::QuestDeleted, QuestDeleted>();
    AddHandler<AB::Packets::Server::QuestRewarded, QuestRewarded>();
    AddHandler<AB::Packets::Server::PlayerInfo, PlayerInfo>();
}

void ProtocolGame::Login(const std::string& accountUuid,
    const std::string& authToken, const std::string& charUuid,
    const std::string& mapUuid,
    const std::string& instanceUuid,
    const std::string& host, uint16_t port)
{
    accountUuid_ = accountUuid;
    authToken_ = authToken;
    charUuid_ = charUuid;
    charUuid_ = charUuid;
    mapUuid_ = mapUuid;
    instanceUuid_ = instanceUuid;

    Connect(host, port);
}

void ProtocolGame::OnConnect()
{
    firstRevc_ = true;
    Protocol::OnConnect();

    // Login packet uses the default key
    SetEncKey(AB::ENC_KEY);
    SendLoginPacket();

    Receive();
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
        ProtocolError(AB::Errors::ErrorException);
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

    ServerPacketType opCode = NoError;
    // One such message contains a variable number of packets and then some padding bytes.
    while (!message.Eof())
    {
        ServerPacketType prevCode = opCode;
        opCode = static_cast<ServerPacketType>(message.Get<uint8_t>());

        if (packetHandlers_.Exists(opCode))
        {
            packetHandlers_.Call(opCode, message);
            continue;
        }

        // There are some special codes we handle separately
        switch (opCode)
        {
        case KeyExchange:
            ParseKeyExchange(message);
            break;
        case Error:
        {
            auto packet = AB::Packets::Get<AB::Packets::Server::ProtocolError>(message);
            if (packet.code != 0)
                ProtocolError(packet.code);
            break;
        }
        case GameStart:
            // This is not interesting for the client
            AB::Packets::Get<AB::Packets::Server::GameStart>(message);
            break;
        case GamePong:
        {
            auto packet = AB::Packets::Get<AB::Packets::Server::Pong>(message);
            // Clock difference between client and server
            clockDiff_ = static_cast<int64_t>(packet.clockDiff);
            // Round trip time
            lastPing_ = static_cast<int>(AbTick() - pingTick_);
            receiver_.OnPong(lastPing_);
            break;
        }
        case GameUpdate:
        {
            auto packet = AB::Packets::Get<AB::Packets::Server::GameUpdate>(message);
            updateTick_ = packet.tick;
            break;
        }
        case CodeLast:
            // Padding bytes, i.e. end of message
            return;
        default:
        {
            std::stringstream ss2;
            ss2 << "ProtocolGame::ParseMessage(): Unknown packet code " << static_cast<int>(opCode) <<
                " last code " << static_cast<int>(prevCode) <<
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

void ProtocolGame::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolGame::ProtocolIdentifier);

    AB::Packets::Client::GameLogin packet;
    packet.clientOs = AB::CLIENT_OS_CURRENT;
    packet.protocolVersion = AB::PROTOCOL_VERSION;
    const DH_KEY& key = keys_.GetPublickKey();
    for (int i = 0; i < DH_KEY_LENGTH; ++i)
        packet.key[i] = key[i];
    packet.accountUuid = accountUuid_;
    packet.authToken = authToken_;
    packet.charUuid = charUuid_;
    packet.mapUuid = mapUuid_;
    packet.instanceUuid = instanceUuid_;
    AB::Packets::Add(packet, *msg);
    Send(std::move(msg));
}

void ProtocolGame::Logout()
{
    loggingOut_ = true;
    AB::Packets::Client::Logout packet;
    SendPacket(AB::GameProtocol::PacketTypeLogout, packet);
}

void ProtocolGame::Ping()
{
    pingTick_ = AbTick();
    AB::Packets::Client::Ping packet = {
        pingTick_
    };
    SendPacket(AB::GameProtocol::PacketTypePing, packet);
}

void ProtocolGame::ChangeMap(const std::string& mapUuid)
{
    AB::Packets::Client::ChangeMap packet = {
        mapUuid
    };
    SendPacket(AB::GameProtocol::PacketTypeChangeMap, packet);
}

void ProtocolGame::GetMailHeaders()
{
    AB::Packets::Client::GetMailHeaders packet;
    SendPacket(AB::GameProtocol::PacketTypeGetMailHeaders, packet);
}

void ProtocolGame::GetInventory()
{
    AB::Packets::Client::GetInventory packet;
    SendPacket(AB::GameProtocol::PacketTypeGetInventory, packet);
}

void ProtocolGame::InventoryStoreItem(uint16_t pos)
{
    AB::Packets::Client::InventoryStoreItem packet = {
        pos
    };
    SendPacket(AB::GameProtocol::PacketTypeInventoryStoreInChest, packet);
}

void ProtocolGame::InventoryDestroyItem(uint16_t pos)
{
    AB::Packets::Client::InventoryDestroyItem packet = {
        pos
    };
    SendPacket(AB::GameProtocol::PacketTypeInventoryDestroyItem, packet);
}

void ProtocolGame::InventoryDropItem(uint16_t pos)
{
    AB::Packets::Client::InventoryDropItem packet = {
        pos
    };
    SendPacket(AB::GameProtocol::PacketTypeInventoryDropItem, packet);
}

void ProtocolGame::GetChest()
{
    AB::Packets::Client::GetChest packet;
    SendPacket(AB::GameProtocol::PacketTypeGetChest, packet);
}

void ProtocolGame::ChestDestroyItem(uint16_t pos)
{
    AB::Packets::Client::ChestDestroyItem packet = {
        pos
    };
    SendPacket(AB::GameProtocol::PacketTypeChestDestroyItem, packet);
}

void ProtocolGame::GetMail(const std::string& mailUuid)
{
    AB::Packets::Client::GetMail packet = {
        mailUuid
    };
    SendPacket(AB::GameProtocol::PacketTypeGetMail, packet);
}

void ProtocolGame::DeleteMail(const std::string& mailUuid)
{
    AB::Packets::Client::DeleteMail packet = {
        mailUuid
    };
    SendPacket(AB::GameProtocol::PacketTypeDeleteMail, packet);
}

void ProtocolGame::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    AB::Packets::Client::SendMail packet = {
        recipient,
        subject,
        body
    };
    SendPacket(AB::GameProtocol::PacketTypeSendMail, packet);
}

void ProtocolGame::GetPlayerInfoByName(const std::string& name, uint32_t fields)
{
    AB::Packets::Client::GetPlayerInfoByName packet = {
        name,
        fields
    };
    SendPacket(AB::GameProtocol::PacketTypeGetPlayerInfoByName, packet);
}

void ProtocolGame::GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields)
{
    AB::Packets::Client::GetPlayerInfoByAccount packet = {
        accountUuid,
        fields
    };
    SendPacket(AB::GameProtocol::PacketTypeGetPlayerInfoByAccount, packet);
}

void ProtocolGame::Move(uint8_t direction)
{
    AB::Packets::Client::Move packet = {
        direction
    };
    SendPacket(AB::GameProtocol::PacketTypeMove, packet);
}

void ProtocolGame::Turn(uint8_t direction)
{
    AB::Packets::Client::Turn packet = {
        direction
    };
    SendPacket(AB::GameProtocol::PacketTypeTurn, packet);
}

void ProtocolGame::SetDirection(float rad)
{
    AB::Packets::Client::SetDirection packet = {
        rad
    };
    SendPacket(AB::GameProtocol::PacketTypeSetDirection, packet);
}

void ProtocolGame::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    AB::Packets::Client::ClickObject packet = {
        sourceId,
        targetId
    };
    SendPacket(AB::GameProtocol::PacketTypeClickObject, packet);
}

void ProtocolGame::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    AB::Packets::Client::SelectObject packet = {
        sourceId,
        targetId
    };
    SendPacket(AB::GameProtocol::PacketTypeSelect, packet);
}

void ProtocolGame::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    AB::Packets::Client::Command packet = {
        type,
        data
    };
    SendPacket(AB::GameProtocol::PacketTypeCommand, packet);
}

void ProtocolGame::GotoPos(const Vec3& pos)
{
    AB::Packets::Client::GotoPos packet = {
        { pos.x, pos.y, pos.z }
    };
    SendPacket(AB::GameProtocol::PacketTypeGoto, packet);
}

void ProtocolGame::Follow(uint32_t targetId, bool ping)
{
    AB::Packets::Client::Follow packet = {
        targetId,
        ping
    };
    SendPacket(AB::GameProtocol::PacketTypeFollow, packet);
}

void ProtocolGame::UseSkill(uint32_t index, bool ping)
{
    AB::Packets::Client::UseSkill packet = {
        static_cast<uint8_t>(index),
        ping
    };
    SendPacket(AB::GameProtocol::PacketTypeUseSkill, packet);
}

void ProtocolGame::Attack(bool ping)
{
    AB::Packets::Client::Attack packet = {
        ping
    };
    SendPacket(AB::GameProtocol::PacketTypeAttack, packet);
}

void ProtocolGame::Cancel()
{
    AB::Packets::Client::Cancel packet = { };
    SendPacket(AB::GameProtocol::PacketTypeCancel, packet);
}

void ProtocolGame::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    AB::Packets::Client::SetPlayerState packet = {
        static_cast<uint8_t>(newState)
    };
    SendPacket(AB::GameProtocol::PacketTypeSetState, packet);
}

void ProtocolGame::PartyInvitePlayer(uint32_t targetId)
{
    AB::Packets::Client::PartyInvitePlayer packet = {
        targetId
    };
    SendPacket(AB::GameProtocol::PacketTypePartyInvitePlayer, packet);
}

void ProtocolGame::PartyKickPlayer(uint32_t targetId)
{
    AB::Packets::Client::PartyKickPlayer packet = {
        targetId
    };
    SendPacket(AB::GameProtocol::PacketTypePartyKickPlayer, packet);
}

void ProtocolGame::PartyLeave()
{
    AB::Packets::Client::PartyLeave packet = { };
    SendPacket(AB::GameProtocol::PacketTypePartyLeave, packet);
}

void ProtocolGame::PartyAcceptInvite(uint32_t inviterId)
{
    AB::Packets::Client::PartyAcceptInvite packet = {
        inviterId
    };
    SendPacket(AB::GameProtocol::PacketTypePartyAcceptInvite, packet);
}

void ProtocolGame::PartyRejectInvite(uint32_t inviterId)
{
    AB::Packets::Client::PartyRejectInvite packet = {
        inviterId
    };
    SendPacket(AB::GameProtocol::PacketTypePartyRejectInvite, packet);
}

void ProtocolGame::PartyGetMembers(uint32_t partyId)
{
    AB::Packets::Client::PartyGetMembers packet = {
        partyId
    };
    SendPacket(AB::GameProtocol::PacektTypeGetPartyMembers, packet);
}

void ProtocolGame::QueueMatch()
{
    AB::Packets::Client::QueueMatch packet = { };
    SendPacket(AB::GameProtocol::PacketTypeQueue, packet);
}

void ProtocolGame::UnqueueMatch()
{
    AB::Packets::Client::UnqueueMatch packet = { };
    SendPacket(AB::GameProtocol::PacketTypeUnqueue, packet);
}

void ProtocolGame::AddFriend(const std::string& name, AB::Entities::FriendRelation relation)
{
    AB::Packets::Client::AddFriend packet = {
        name,
        relation
    };
    SendPacket(AB::GameProtocol::PacketTypeAddFriend, packet);
}

void ProtocolGame::RemoveFriend(const std::string& accountUuid)
{
    AB::Packets::Client::RemoveFriend packet = {
        accountUuid
    };
    SendPacket(AB::GameProtocol::PacketTypeRemoveFriend, packet);
}

void ProtocolGame::UpdateFriendList()
{
    AB::Packets::Client::UpdateFriendList packet = { };
    SendPacket(AB::GameProtocol::PacketTypeGetFriendList, packet);
}

void ProtocolGame::SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status)
{
    AB::Packets::Client::SetOnlineStatus packet = {
        status
    };
    SendPacket(AB::GameProtocol::PacketTypeSetOnlineStatus, packet);
}

}
