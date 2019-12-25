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
            ParseError(message);
            break;
        case ChangeInstance:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ChangeInstance>(message));
            break;
        case GameStart:
            ParseGameStart(message);
            break;
        case GameEnter:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::EnterWorld>(message));
            break;
        case GamePong:
            ParsePong(message);
            break;
        case PlayerError:
            ParseGameError(message);
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
            ParseUpdate(message);
            break;
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
            ParseObjectStateChange(message);
            break;
        case GameObjectMoveSpeedChange:
            receiver_.OnPacket(updateTick_, AB::Packets::Get<AB::Packets::Server::ObjectSpeedChanged>(message));
            break;
        case GameObjectSelectTarget:
            ParseObjectSelected(message);
            break;
        case GameObjectSkillFailure:
            ParseObjectSkillFailure(message);
            break;
        case GameObjectUseSkill:
            ParseObjectUseSkill(message);
            break;
        case GameObjectEndUseSkill:
            ParseObjectEndUseSkill(message);
            break;
        case GameObjectAttackFailure:
            ParseObjectAttackFailure(message);
            break;
        case GameObjectPingTarget:
            ParseObjectPingTarget(message);
            break;
        case GameObjectEffectAdded:
            ParseObjectEffectAdded(message);
            break;
        case GameObjectEffectRemoved:
            ParseObjectEffectRemoved(message);
            break;
        case GameObjectDamaged:
            ParseObjectDamaged(message);
            break;
        case GameObjectHealed:
            ParseObjectHealed(message);
            break;
        case GameObjectProgress:
            ParseObjectProgress(message);
            break;
        case GameObjectDropItem:
            ParseObjectDroppedItem(message);
            break;
        case GameObjectSetPosition:
            ParseObjectSetPosition(message);
            break;
        case ServerMessage:
            ParseServerMessage(message);
            break;
        case ChatMessage:
            ParseChatMessage(message);
            break;
        case PartyPlayerInvited:
            ParsePartyPlayerInvited(message);
            break;
        case PartyPlayerRemoved:
            ParsePartyPlayerRemoved(message);
            break;
        case PartyPlayerAdded:
            ParsePartyPlayerAdded(message);
            break;
        case PartyInviteRemoved:
            ParsePartyInviteRemoved(message);
            break;
        case PartyResigned:
            ParsePartyResigned(message);
            break;
        case PartyDefeated:
            ParsePartyDefeated(message);
            break;
        case PartyInfoMembers:
            ParsePartyInfoMembers(message);
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

void ProtocolGame::ParseObjectStateChange(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    AB::GameProtocol::CreatureState state = static_cast<AB::GameProtocol::CreatureState>(message.Get<uint8_t>());

    receiver_.OnObjectStateChange(updateTick_, objectId, state);
}

void ProtocolGame::ParseObjectSelected(InputMessage& message)
{
    uint32_t sourceId = message.Get<uint32_t>();
    uint32_t targetId = message.Get<uint32_t>();

    receiver_.OnObjectSelected(updateTick_, sourceId, targetId);
}

void ProtocolGame::ParseObjectSkillFailure(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    int skillIndex = static_cast<int>(message.Get<int8_t>());
    AB::GameProtocol::SkillError skillError = static_cast<AB::GameProtocol::SkillError>(message.Get<uint8_t>());

    receiver_.OnObjectSkillFailure(updateTick_, objectId, skillIndex, skillError);
}

void ProtocolGame::ParseObjectUseSkill(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    // Index in the skill bar, 1 based
    int skillIndex = static_cast<int>(message.Get<uint8_t>());
    uint16_t energy = message.Get<uint16_t>();
    uint16_t adrenaline = message.Get<uint16_t>();
    uint16_t activation = message.Get<uint16_t>();
    uint16_t overcast = message.Get<uint16_t>();
    uint16_t hpScarifies = message.Get<uint16_t>();

    receiver_.OnObjectUseSkill(updateTick_, objectId, skillIndex, energy, adrenaline, activation, overcast, hpScarifies);
}

void ProtocolGame::ParseObjectEndUseSkill(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    int skillIndex = static_cast<int>(message.Get<uint8_t>());
    uint32_t recharge = message.Get<uint32_t>();

    receiver_.OnObjectEndUseSkill(updateTick_, objectId, skillIndex, recharge);
}

void ProtocolGame::ParseObjectAttackFailure(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    AB::GameProtocol::AttackError attackError = static_cast<AB::GameProtocol::AttackError>(message.Get<uint8_t>());

    receiver_.OnObjectAttackFailure(updateTick_, objectId, attackError);
}

void ProtocolGame::ParseObjectPingTarget(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    uint32_t targetId = message.Get<uint32_t>();
    AB::GameProtocol::ObjectCallType type = static_cast<AB::GameProtocol::ObjectCallType>(message.Get<uint8_t>());
    int skillIndex = message.Get<int8_t>();

    receiver_.OnObjectPingTarget(updateTick_, objectId, targetId, type, skillIndex);
}

void ProtocolGame::ParseObjectEffectAdded(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    uint32_t effectIndex = message.Get<uint32_t>();
    uint32_t ticks = message.Get<uint32_t>();

    receiver_.OnObjectEffectAdded(updateTick_, objectId, effectIndex, ticks);
}

void ProtocolGame::ParseObjectEffectRemoved(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    uint32_t effectIndex = message.Get<uint32_t>();

    receiver_.OnObjectEffectRemoved(updateTick_, objectId, effectIndex);
}

void ProtocolGame::ParseObjectDamaged(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    uint32_t sourceId = message.Get<uint32_t>();
    uint16_t index = message.Get<uint16_t>();
    uint8_t damageType = message.Get<uint8_t>();
    int16_t damageValue = message.Get<int16_t>();

    receiver_.OnObjectDamaged(updateTick_, objectId, sourceId, index, damageType, damageValue);
}

void ProtocolGame::ParseObjectHealed(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    uint32_t healerId = message.Get<uint32_t>();
    uint16_t index = message.Get<uint16_t>();
    int16_t healValue = message.Get<int16_t>();

    receiver_.OnObjectHealed(updateTick_, objectId, healerId, index, healValue);
}

void ProtocolGame::ParseObjectProgress(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    AB::GameProtocol::ObjectProgressType type = static_cast<AB::GameProtocol::ObjectProgressType>(message.Get<uint8_t>());
    int value = static_cast<int>(message.Get<int16_t>());

    receiver_.OnObjectProgress(updateTick_, objectId, type, value);
}

void ProtocolGame::ParseObjectDroppedItem(InputMessage& message)
{
    uint32_t dropperId = message.Get<uint32_t>();
    uint32_t targetId = message.Get<uint32_t>();
    uint32_t itemId = message.Get<uint32_t>();
    uint32_t itemIndex = message.Get<uint32_t>();
    uint32_t count = message.Get<uint32_t>();
    uint16_t value = message.Get<uint16_t>();

    receiver_.OnObjectDroppedItem(updateTick_, dropperId, targetId, itemId, itemIndex, count, value);
}

void ProtocolGame::ParseObjectSetPosition(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    Vec3 pos = {
        message.Get<float>(),
        message.Get<float>(),
        message.Get<float>()
    };

    receiver_.OnObjectSetPosition(updateTick_, objectId, pos);
}

void ProtocolGame::ParseServerMessage(InputMessage& message)
{
    AB::GameProtocol::ServerMessageType type =
        static_cast<AB::GameProtocol::ServerMessageType>(message.Get<uint8_t>());
    std::string sender = message.Get<std::string>();
    std::string data = message.Get<std::string>();

    receiver_.OnServerMessage(updateTick_, type, sender, data);
}

void ProtocolGame::ParseChatMessage(InputMessage& message)
{
    AB::GameProtocol::ChatMessageChannel type =
        static_cast<AB::GameProtocol::ChatMessageChannel>(message.Get<uint8_t>());
    uint32_t senderId = message.Get<uint32_t>();
    std::string sender = message.Get<std::string>();
    std::string data = message.Get<std::string>();

    receiver_.OnChatMessage(updateTick_, type, senderId, sender, data);
}

void ProtocolGame::ParsePartyPlayerInvited(InputMessage& message)
{
    uint32_t sourceId = message.Get<uint32_t>();         // Inviter (source)
    uint32_t targetId = message.Get<uint32_t>();         // Invitee (target)
    uint32_t partyId = message.Get<uint32_t>();

    receiver_.OnPartyInvited(updateTick_, sourceId, targetId, partyId);
}

void ProtocolGame::ParsePartyPlayerRemoved(InputMessage& message)
{
    uint32_t sourceId = message.Get<uint32_t>();
    uint32_t targetId = message.Get<uint32_t>();
    uint32_t partyId = message.Get<uint32_t>();

    receiver_.OnPartyRemoved(updateTick_, sourceId, targetId, partyId);
}

void ProtocolGame::ParsePartyPlayerAdded(InputMessage& message)
{
    uint32_t acceptorId = message.Get<uint32_t>();
    uint32_t leaderId = message.Get<uint32_t>();
    uint32_t partyId = message.Get<uint32_t>();

    receiver_.OnPartyAdded(updateTick_, acceptorId, leaderId, partyId);
}

void ProtocolGame::ParsePartyInviteRemoved(InputMessage& message)
{
    uint32_t sourceId = message.Get<uint32_t>();
    uint32_t targetId = message.Get<uint32_t>();
    uint32_t partyId = message.Get<uint32_t>();

    receiver_.OnPartyInviteRemoved(updateTick_, sourceId, targetId, partyId);
}

void ProtocolGame::ParsePartyResigned(InputMessage& message)
{
    uint32_t partyId = message.Get<uint32_t>();

    receiver_.OnPartyResigned(updateTick_, partyId);
}

void ProtocolGame::ParsePartyDefeated(InputMessage& message)
{
    uint32_t partyId = message.Get<uint32_t>();

    receiver_.OnPartyDefeated(updateTick_, partyId);
}

void ProtocolGame::ParsePartyInfoMembers(InputMessage& message)
{
    uint32_t partyId = message.Get<uint32_t>();
    size_t count = message.Get<uint8_t>();
    std::vector<uint32_t> members;
    members.resize(count);
    for (size_t i = 0; i < count; i++)
    {
        members[i] = message.Get<uint32_t>();
    }

    receiver_.OnPartyInfoMembers(partyId, members);
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

void ProtocolGame::ParseUpdate(InputMessage& message)
{
    updateTick_ = message.Get<int64_t>();
}

void ProtocolGame::ParsePong(InputMessage& message)
{
    int32_t diff = message.Get<int32_t>();
    // Clock difference between client and server
    clockDiff_ = static_cast<int64_t>(diff);
    // Round trip time
    lastPing_ = static_cast<int>(AbTick() - pingTick_);

    receiver_.OnPong(lastPing_);
}

void ProtocolGame::ParseGameError(InputMessage& message)
{
    AB::GameProtocol::PlayerErrorValue error = static_cast<AB::GameProtocol::PlayerErrorValue>(message.Get<uint8_t>());

    receiver_.OnPlayerError(updateTick_, error);
}

void ProtocolGame::ParseError(InputMessage& message)
{
    uint8_t error = message.Get<uint8_t>();
    if (error != 0)
        ProtocolError(error);
}

void ProtocolGame::ParseGameStart(InputMessage& message)
{
    // We must read it
    /* int64_t startTick = */ message.Get<int64_t>();
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
