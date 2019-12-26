#include "stdafx.h"
#include "ProtocolGame.h"
#include "TimeUtils.h"
#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>
#include <set>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

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

        switch (opCode)
        {
        case KeyExchange:
            ParseKeyExchange(message);
            break;
        case ServerJoined:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ServerJoined>(message));
            break;
        case ServerLeft:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ServerLeft>(message));
            break;
        case Error:
        {
            auto packet = AB::Packets::Get<AB::Packets::Server::ProtocolError>(message);
            if (packet.code != 0)
                ProtocolError(packet.code);
            break;
        }
        case ChangeInstance:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ChangeInstance>(message));
            break;
        case GameStart:
            /* auto packet = */ AB::Packets::Get<AB::Packets::Server::GameStart>(message);
            break;
        case GameEnter:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::EnterWorld>(message));
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
        case PlayerError:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::GameError>(message));
            break;
        case PlayerAutoRun:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::PlayerAutorun>(message));
            break;
        case MailHeaders:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::MailHeaders>(message));
            break;
        case MailComplete:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::MailComplete>(message));
            break;
        case InventoryContent:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::InventoryContent>(message));
            break;
        case InventoryItemUpdate:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::InventoryItemUpdate>(message));
            break;
        case InventoryItemDelete:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::InventoryItemDelete>(message));
            break;
        case ChestContent:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ChestContent>(message));
            break;
        case ChestItemUpdate:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ChestItemUpdate>(message));
            break;
        case ChestItemDelete:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ChestItemDelete>(message));
            break;
        case GameUpdate:
        {
            auto packet = AB::Packets::Get<AB::Packets::Server::GameUpdate>(message);
            updateTick_ = packet.tick;
            break;
        }
        case GameSpawnObjectExisting:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectSpawnExisting>(message));
            break;
        case GameSpawnObject:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectSpawn>(message));
            break;
        case GameLeaveObject:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectDespawn>(message));
            break;
        case GameObjectPositionChange:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectPosUpdate>(message));
            break;
        case GameObjectRotationChange:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectRotationUpdate>(message));
            break;
        case GameObjectStateChange:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectStateChanged>(message));
            break;
        case GameObjectMoveSpeedChange:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectSpeedChanged>(message));
            break;
        case GameObjectSelectTarget:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectTargetSelected>(message));
            break;
        case GameObjectSkillFailure:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectSkillFailure>(message));
            break;
        case GameObjectUseSkill:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectUseSkill>(message));
            break;
        case GameObjectEndUseSkill:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectSkillSuccess>(message));
            break;
        case GameObjectAttackFailure:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectAttackFailure>(message));
            break;
        case GameObjectPingTarget:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectPingTarget>(message));
            break;
        case GameObjectEffectAdded:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectEffectAdded>(message));
            break;
        case GameObjectEffectRemoved:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectEffectRemoved>(message));
            break;
        case GameObjectDamaged:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectDamaged>(message));
            break;
        case GameObjectHealed:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectHealed>(message));
            break;
        case GameObjectProgress:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectProgress>(message));
            break;
        case GameObjectDropItem:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectDroppedItem>(message));
            break;
        case GameObjectSetPosition:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectSetPosition>(message));
            break;
        case ServerMessage:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ServerMessage>(message));
            break;
        case ChatMessage:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ChatMessage>(message));
            break;
        case PartyPlayerInvited:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::PartyPlayerInvited>(message));
            break;
        case PartyPlayerRemoved:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::PartyPlayerRemoved>(message));
            break;
        case PartyPlayerAdded:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::PartyPlayerAdded>(message));
            break;
        case PartyInviteRemoved:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::PartyInviteRemoved>(message));
            break;
        case PartyResigned:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::PartyResigned>(message));
            break;
        case PartyDefeated:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::PartyDefeated>(message));
            break;
        case PartyInfoMembers:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::PartyMembersInfo>(message));
            break;
        case GameObjectResourceChange:
            ParseResourceChanged(message);
            break;
        case DialogTrigger:
            ParseDialogTrigger(message);
            break;
        case FriendList:
            ParseFriendList(message);
            break;
        case FriendAdded:
            ParseFriendAdded(message);
            break;
        case FriendRemoved:
            ParseFriendRemoved(message);
            break;
        case GuildInfo:
            ParseGuildInfo(message);
            break;
        case GuildMemberList:
            ParseGuildMemberList(message);
            break;
        case QuestSelectionDialogTrigger:
            ParseQuestSelectionDialogTrigger(message);
            break;
        case QuestDialogTrigger:
            ParseQuestDialogTrigger(message);
            break;
        case QuestNpcHasQuest:
            ParseQuestNpcHasQuest(message);
            break;
        case QuestDeleted:
            ParseQuestDeleted(message);
            break;
        case QuestRewarded:
            ParseQuestRewarded(message);
            break;
        case PlayerInfo:
            ParsePlayerInfo(message);
            break;
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

void ProtocolGame::ParseResourceChanged(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    AB::GameProtocol::ResourceType resType = static_cast<AB::GameProtocol::ResourceType>(message.Get<uint8_t>());
    switch (resType)
    {
    case AB::GameProtocol::ResourceTypeHealth:
    case AB::GameProtocol::ResourceTypeEnergy:
    case AB::GameProtocol::ResourceTypeAdrenaline:
    case AB::GameProtocol::ResourceTypeOvercast:
    case AB::GameProtocol::ResourceTypeMaxHealth:
    case AB::GameProtocol::ResourceTypeMaxEnergy:
    {
        int16_t value = message.Get<int16_t>();

        receiver_.OnResourceChanged(updateTick_, objectId, resType, value);
        break;
    }
    case AB::GameProtocol::ResourceTypeHealthRegen:
    case AB::GameProtocol::ResourceTypeEnergyRegen:
    case AB::GameProtocol::ResourceTypeMorale:
    {
        int8_t value = message.Get<int8_t>();

        receiver_.OnResourceChanged(updateTick_, objectId, resType, static_cast<int16_t>(value));
        break;
    }
    }
}

void ProtocolGame::ParseDialogTrigger(InputMessage& message)
{
    uint32_t dialogId = message.Get<uint32_t>();

    receiver_.OnDialogTrigger(updateTick_, dialogId);
}

void ProtocolGame::ParseFriendList(InputMessage& message)
{
    std::vector<std::string> friends;
    size_t count = message.Get<uint16_t>();
    friends.reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
        friends.push_back(message.Get<std::string>());
    }

    receiver_.OnFriendList(updateTick_, friends);
}

void ProtocolGame::ParseFriendAdded(InputMessage& message)
{
    std::string accountUuid = message.Get<std::string>();
    RelatedAccount::Relation rel = static_cast<RelatedAccount::Relation>(message.Get<uint8_t>());
    receiver_.OnFriendAdded(updateTick_, accountUuid, rel);
}

void ProtocolGame::ParseFriendRemoved(InputMessage& message)
{
    std::string accountUuid = message.Get<std::string>();
    RelatedAccount::Relation rel = static_cast<RelatedAccount::Relation>(message.Get<uint8_t>());
    receiver_.OnFriendRemoved(updateTick_, accountUuid, rel);
}

void ProtocolGame::ParseGuildInfo(InputMessage& message)
{
    AB::Entities::Guild guild;
    guild.uuid = message.Get<std::string>();
    guild.name = message.Get<std::string>();
    guild.tag = message.Get<std::string>();
    guild.creation = message.Get<int64_t>();
    guild.creatorAccountUuid = message.Get<std::string>();

    receiver_.OnGuildInfo(updateTick_, guild);
}

void ProtocolGame::ParseGuildMemberList(InputMessage& message)
{
    std::vector<std::string> members;
    size_t count = message.Get<uint16_t>();
    members.reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
        members.push_back(message.Get<std::string>());
    }

    receiver_.OnGuildMemberList(updateTick_, members);
}

void ProtocolGame::ParsePlayerInfo(InputMessage& message)
{
    uint32_t fields = message.Get<uint32_t>();
    RelatedAccount frnd;
    frnd.fields = fields;
    frnd.accountUuid = message.Get<std::string>();
    if (fields & AB::GameProtocol::PlayerInfoFieldName)
        frnd.nickName = message.Get<std::string>();
    if (fields & AB::GameProtocol::PlayerInfoFieldRelation)
        frnd.relation = static_cast<RelatedAccount::Relation>(message.Get<uint8_t>());
    if (fields & AB::GameProtocol::PlayerInfoFieldOnlineStatus)
        frnd.status = static_cast<RelatedAccount::Status>(message.Get<uint8_t>());
    if (fields & AB::GameProtocol::PlayerInfoFieldCurrentName)
        frnd.currentName = message.Get<std::string>();
    if (fields & AB::GameProtocol::PlayerInfoFieldCurrentMap)
        frnd.currentMap = message.Get<std::string>();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildGuid)
        frnd.guildUuid = message.Get<std::string>();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildRole)
        frnd.guildRole = static_cast<RelatedAccount::GuildRole>(message.Get<uint8_t>());
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildInviteName)
        frnd.guildInviteName = message.Get<std::string>();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildInvited)
        frnd.invited = message.Get<int64_t>();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildJoined)
        frnd.joined = message.Get<int64_t>();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildExpires)
        frnd.expires = message.Get<int64_t>();

    receiver_.OnPlayerInfo(updateTick_, frnd);
}

void ProtocolGame::ParseQuestSelectionDialogTrigger(InputMessage& message)
{
    uint32_t count = message.Get<uint8_t>();
    std::set<uint32_t> quests;
    for (uint32_t i = 0; i < count; ++i)
        quests.emplace(message.Get<uint32_t>());
    receiver_.OnQuestSelectionDialogTrigger(updateTick_, quests);
}

void ProtocolGame::ParseQuestDialogTrigger(InputMessage& message)
{
    uint32_t questIndex = message.Get<uint32_t>();
    receiver_.OnQuestDialogTrigger(updateTick_, questIndex);
}

void ProtocolGame::ParseQuestNpcHasQuest(InputMessage& message)
{
    uint32_t npcId = message.Get<uint32_t>();
    bool hasQuest = message.Get<bool>();
    receiver_.OnNpcHasQuest(updateTick_, npcId, hasQuest);
}

void ProtocolGame::ParseQuestDeleted(InputMessage& message)
{
    uint32_t index = message.Get<uint32_t>();
    bool deleted = message.Get<bool>();
    receiver_.OnQuestDeleted(updateTick_, index, deleted);
}

void ProtocolGame::ParseQuestRewarded(InputMessage& message)
{
//    auto packet = AB::Packets::Server::Get<AB::Packets::Server::QuestRewarded>(message);

    uint32_t index = message.Get<uint32_t>();
    bool rewarded = message.Get<bool>();
    receiver_.OnQuestRewarded(updateTick_, index, rewarded);
}

void ProtocolGame::LogMessage(const std::string& message)
{
    receiver_.OnLog(message);
}

void ProtocolGame::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolGame::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    const DH_KEY& key = keys_.GetPublickKey();
    for (int i = 0; i < DH_KEY_LENGTH; ++i)
        msg->Add<uint8_t>(key[i]);
    msg->AddString(accountUuid_);
    msg->AddString(authToken_);
    msg->AddString(charUuid_);
    msg->AddString(mapUuid_);
    msg->AddString(instanceUuid_);
    Send(std::move(msg));
}

void ProtocolGame::Logout()
{
    loggingOut_ = true;
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeLogout);
    Send(std::move(msg));
}

void ProtocolGame::Ping()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePing);
    pingTick_ = AbTick();
    msg->Add<int64_t>(pingTick_);
    Send(std::move(msg));
}

void ProtocolGame::ChangeMap(const std::string& mapUuid)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeChangeMap);
    msg->AddString(mapUuid);
    Send(std::move(msg));
}

void ProtocolGame::GetMailHeaders()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetMailHeaders);
    Send(std::move(msg));
}

void ProtocolGame::GetInventory()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetInventory);
    Send(std::move(msg));
}

void ProtocolGame::InventoryStoreItem(uint16_t pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeInventoryStoreInChest);
    msg->Add<uint16_t>(pos);
    Send(std::move(msg));
}

void ProtocolGame::InventoryDestroyItem(uint16_t pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeInventoryDestroyItem);
    msg->Add<uint16_t>(pos);
    Send(std::move(msg));
}

void ProtocolGame::InventoryDropItem(uint16_t pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeInventoryDropItem);
    msg->Add<uint16_t>(pos);
    Send(std::move(msg));
}

void ProtocolGame::GetChest()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetChest);
    Send(std::move(msg));
}

void ProtocolGame::ChestDestroyItem(uint16_t pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeChestDestroyItem);
    msg->Add<uint16_t>(pos);
    Send(std::move(msg));
}

void ProtocolGame::GetMail(const std::string& mailUuid)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetMail);
    msg->AddString(mailUuid);
    Send(std::move(msg));
}

void ProtocolGame::DeleteMail(const std::string& mailUuid)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeDeleteMail);
    msg->AddString(mailUuid);
    Send(std::move(msg));
}

void ProtocolGame::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSendMail);
    msg->AddString(recipient);
    msg->AddString(subject);
    msg->AddString(body);
    Send(std::move(msg));
}

void ProtocolGame::GetPlayerInfoByName(const std::string& name, uint32_t fields)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetPlayerInfoByName);
    msg->AddString(name);
    msg->Add<uint32_t>(fields);
    Send(std::move(msg));
}

void ProtocolGame::GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetPlayerInfoByAccount);
    msg->AddString(accountUuid);
    msg->Add<uint32_t>(fields);
    Send(std::move(msg));
}

void ProtocolGame::Move(uint8_t direction)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeMove);
    msg->Add<uint8_t>(direction);
    Send(std::move(msg));
}

void ProtocolGame::Turn(uint8_t direction)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeTurn);
    msg->Add<uint8_t>(direction);
    Send(std::move(msg));
}

void ProtocolGame::SetDirection(float rad)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSetDirection);
    msg->Add<float>(rad);
    Send(std::move(msg));
}

void ProtocolGame::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeClickObject);
    msg->Add<uint32_t>(sourceId);
    msg->Add<uint32_t>(targetId);
    Send(std::move(msg));
}

void ProtocolGame::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSelect);
    msg->Add<uint32_t>(sourceId);
    msg->Add<uint32_t>(targetId);
    Send(std::move(msg));
}

void ProtocolGame::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeCommand);
    msg->Add<uint8_t>(type);
    msg->AddString(data);
    Send(std::move(msg));
}

void ProtocolGame::GotoPos(const Vec3& pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGoto);
    msg->Add<float>(pos.x);
    msg->Add<float>(pos.y);
    msg->Add<float>(pos.z);
    Send(std::move(msg));
}

void ProtocolGame::Follow(uint32_t targetId, bool ping)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeFollow);
    msg->Add<uint32_t>(targetId);
    msg->Add<bool>(ping);
    Send(std::move(msg));
}

void ProtocolGame::UseSkill(uint32_t index, bool ping)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeUseSkill);
    msg->Add<uint8_t>(static_cast<uint8_t>(index));
    msg->Add<bool>(ping);
    Send(std::move(msg));
}

void ProtocolGame::Attack(bool ping)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeAttack);
    msg->Add<bool>(ping);
    Send(std::move(msg));
}

void ProtocolGame::Cancel()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeCancel);
    Send(std::move(msg));
}

void ProtocolGame::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSetState);
    msg->Add<uint8_t>(static_cast<uint8_t>(newState));
    Send(std::move(msg));
}

void ProtocolGame::PartyInvitePlayer(uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyInvitePlayer);
    msg->Add<uint32_t>(targetId);
    Send(std::move(msg));
}

void ProtocolGame::PartyKickPlayer(uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyKickPlayer);
    msg->Add<uint32_t>(targetId);
    Send(std::move(msg));
}

void ProtocolGame::PartyLeave()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyLeave);
    Send(std::move(msg));
}

void ProtocolGame::PartyAcceptInvite(uint32_t inviterId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyAcceptInvite);
    msg->Add<uint32_t>(inviterId);
    Send(std::move(msg));
}

void ProtocolGame::PartyRejectInvite(uint32_t inviterId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyRejectInvite);
    msg->Add<uint32_t>(inviterId);
    Send(std::move(msg));
}

void ProtocolGame::PartyGetMembers(uint32_t partyId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacektTypeGetPartyMembers);
    msg->Add<uint32_t>(partyId);
    Send(std::move(msg));
}

void ProtocolGame::QueueMatch()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeQueue);
    Send(std::move(msg));
}

void ProtocolGame::UnqueueMatch()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeUnqueue);
    Send(std::move(msg));
}

void ProtocolGame::AddFriend(const std::string& name, AB::Entities::FriendRelation relation)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeAddFriend);
    msg->AddString(name);
    msg->Add<uint8_t>(static_cast<uint8_t>(relation));
    Send(std::move(msg));
}

void ProtocolGame::RemoveFriend(const std::string& accountUuid)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeRemoveFriend);
    msg->AddString(accountUuid);
    Send(std::move(msg));
}

void ProtocolGame::UpdateFriendList()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetFriendList);
    Send(std::move(msg));
}

void ProtocolGame::SetOnlineStatus(RelatedAccount::Status status)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSetOnlineStatus);
    msg->Add<uint8_t>(static_cast<uint8_t>(status));
    Send(std::move(msg));
}

}
