#include "stdafx.h"
#include "ProtocolGame.h"
#include "TimeUtils.h"
#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>

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
    AB::GameProtocol::GameProtocolCodes opCode = AB::GameProtocol::NoError;
    // One such message contains a variable number of packets and then some padding bytes.
    while (!message.Eof())
    {
        AB::GameProtocol::GameProtocolCodes prevCode = opCode;
        opCode = static_cast<AB::GameProtocol::GameProtocolCodes>(message.Get<uint8_t>());

        switch (opCode)
        {
        case AB::GameProtocol::KeyExchange:
            ParseKeyExchange(message);
            break;
        case AB::GameProtocol::ServerJoined:
            ParseServerJoined(message);
            break;
        case AB::GameProtocol::ServerLeft:
            ParseServerLeft(message);
            break;
        case AB::GameProtocol::Error:
            ParseError(message);
            break;
        case AB::GameProtocol::ChangeInstance:
            ParseChangeInstance(message);
            break;
        case AB::GameProtocol::GameStart:
            ParseGameStart(message);
            break;
        case AB::GameProtocol::GameEnter:
            ParseEnterWorld(message);
            break;
        case AB::GameProtocol::GamePong:
            ParsePong(message);
            break;
        case AB::GameProtocol::PlayerError:
            ParseGameError(message);
            break;
        case AB::GameProtocol::PlayerAutoRun:
            ParsePlayerAutoRun(message);
            break;
        case AB::GameProtocol::MailHeaders:
            ParseMailHeaders(message);
            break;
        case AB::GameProtocol::MailComplete:
            ParseMailComplete(message);
            break;
        case AB::GameProtocol::InventoryContent:
            ParseInventoryContent(message);
            break;
        case AB::GameProtocol::InventoryItemUpdate:
            ParseInventoryItemUpdate(message);
            break;
        case AB::GameProtocol::InventoryItemDelete:
            ParseInventoryItemDelete(message);
            break;
        case AB::GameProtocol::ChestContent:
            ParseChestContent(message);
            break;
        case AB::GameProtocol::ChestItemUpdate:
            ParseChestItemUpdate(message);
            break;
        case AB::GameProtocol::ChestItemDelete:
            ParseChestItemDelete(message);
            break;
        case AB::GameProtocol::GameUpdate:
            ParseUpdate(message);
            break;
        case AB::GameProtocol::GameSpawnObjectExisting:
        case AB::GameProtocol::GameSpawnObject:
            ParseObjectSpawn(opCode == AB::GameProtocol::GameSpawnObjectExisting, message);
            break;
        case AB::GameProtocol::GameLeaveObject:
            ParseLeaveObject(message);
            break;
        case AB::GameProtocol::GameObjectPositionChange:
            ParseObjectPosUpdate(message);
            break;
        case AB::GameProtocol::GameObjectRotationChange:
            ParseObjectRotUpdate(message);
            break;
        case AB::GameProtocol::GameObjectStateChange:
            ParseObjectStateChange(message);
            break;
        case AB::GameProtocol::GameObjectMoveSpeedChange:
            ParseObjectSpeedChange(message);
            break;
        case AB::GameProtocol::GameObjectSelectTarget:
            ParseObjectSelected(message);
            break;
        case AB::GameProtocol::GameObjectSkillFailure:
            ParseObjectSkillFailure(message);
            break;
        case AB::GameProtocol::GameObjectUseSkill:
            ParseObjectUseSkill(message);
            break;
        case AB::GameProtocol::GameObjectEndUseSkill:
            ParseObjectEndUseSkill(message);
            break;
        case AB::GameProtocol::GameObjectAttackFailure:
            ParseObjectAttackFailure(message);
            break;
        case AB::GameProtocol::GameObjectPingTarget:
            ParseObjectPingTarget(message);
            break;
        case AB::GameProtocol::GameObjectEffectAdded:
            ParseObjectEffectAdded(message);
            break;
        case AB::GameProtocol::GameObjectEffectRemoved:
            ParseObjectEffectRemoved(message);
            break;
        case AB::GameProtocol::GameObjectDamaged:
            ParseObjectDamaged(message);
            break;
        case AB::GameProtocol::GameObjectHealed:
            ParseObjectHealed(message);
            break;
        case AB::GameProtocol::GameObjectProgress:
            ParseObjectProgress(message);
            break;
        case AB::GameProtocol::GameObjectDropItem:
            ParseObjectDroppedItem(message);
            break;
        case AB::GameProtocol::GameObjectSetPosition:
            ParseObjectSetPosition(message);
            break;
        case AB::GameProtocol::ServerMessage:
            ParseServerMessage(message);
            break;
        case AB::GameProtocol::ChatMessage:
            ParseChatMessage(message);
            break;
        case AB::GameProtocol::PartyPlayerInvited:
            ParsePartyPlayerInvited(message);
            break;
        case AB::GameProtocol::PartyPlayerRemoved:
            ParsePartyPlayerRemoved(message);
            break;
        case AB::GameProtocol::PartyPlayerAdded:
            ParsePartyPlayerAdded(message);
            break;
        case AB::GameProtocol::PartyInviteRemoved:
            ParsePartyInviteRemoved(message);
            break;
        case AB::GameProtocol::PartyResigned:
            ParsePartyResigned(message);
            break;
        case AB::GameProtocol::PartyDefeated:
            ParsePartyDefeated(message);
            break;
        case AB::GameProtocol::PartyInfoMembers:
            ParsePartyInfoMembers(message);
            break;
        case AB::GameProtocol::GameObjectResourceChange:
            ParseResourceChanged(message);
            break;
        case AB::GameProtocol::DialogTrigger:
            ParseDialogTrigger(message);
            break;
        case AB::GameProtocol::FriendList:
            ParseFriendList(message);
            break;
        case AB::GameProtocol::FriendAdded:
            ParseFriendAdded(message);
            break;
        case AB::GameProtocol::FriendRemoved:
            ParseFriendRemoved(message);
            break;
        case AB::GameProtocol::GuildInfo:
            ParseGuildInfo(message);
            break;
        case AB::GameProtocol::GuildMemberList:
            ParseGuildMemberList(message);
            break;
        case AB::GameProtocol::PlayerInfo:
            ParsePlayerInfo(message);
            break;
        case AB::GameProtocol::CodeLast:
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

void ProtocolGame::ParseObjectRotUpdate(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    float rot = message.Get<float>();
    bool manual = message.Get<uint8_t>() != 0;

    receiver_.OnObjectRot(updateTick_, objectId, rot, manual);
}

void ProtocolGame::ParseObjectStateChange(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    AB::GameProtocol::CreatureState state = static_cast<AB::GameProtocol::CreatureState>(message.Get<uint8_t>());

    receiver_.OnObjectStateChange(updateTick_, objectId, state);
}

void ProtocolGame::ParseObjectSpeedChange(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    float speedFactor = message.Get<float>();

    receiver_.OnObjectSpeedChange(updateTick_, objectId, speedFactor);
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
    uint16_t recharge = message.Get<uint16_t>();

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
    int16_t damageValue = message.Get<uint16_t>();

    receiver_.OnObjectDamaged(updateTick_, objectId, sourceId, index, damageType, damageValue);
}

void ProtocolGame::ParseObjectHealed(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    uint32_t healerId = message.Get<uint32_t>();
    uint16_t index = message.Get<uint16_t>();
    int16_t healValue = message.Get<uint16_t>();

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
    std::string sender = message.GetString();
    std::string data = message.GetString();

    receiver_.OnServerMessage(updateTick_, type, sender, data);
}

void ProtocolGame::ParseChatMessage(InputMessage& message)
{
    AB::GameProtocol::ChatMessageChannel type =
        static_cast<AB::GameProtocol::ChatMessageChannel>(message.Get<uint8_t>());
    uint32_t senderId = message.Get<uint32_t>();
    std::string sender = message.GetString();
    std::string data = message.GetString();

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
        friends.push_back(message.GetString());
    }

    receiver_.OnFriendList(updateTick_, friends);
}

void ProtocolGame::ParseFriendAdded(InputMessage& message)
{
    std::string accountUuid = message.GetString();
    RelatedAccount::Relation rel = static_cast<RelatedAccount::Relation>(message.Get<uint8_t>());
    receiver_.OnFriendAdded(updateTick_, accountUuid, rel);
}

void ProtocolGame::ParseFriendRemoved(InputMessage& message)
{
    std::string accountUuid = message.GetString();
    RelatedAccount::Relation rel = static_cast<RelatedAccount::Relation>(message.Get<uint8_t>());
    receiver_.OnFriendRemoved(updateTick_, accountUuid, rel);
}

void ProtocolGame::ParseGuildInfo(InputMessage& message)
{
    AB::Entities::Guild guild;
    guild.uuid = message.GetString();
    guild.name = message.GetString();
    guild.tag = message.GetString();
    guild.creation = message.Get<int64_t>();
    guild.creatorAccountUuid = message.GetString();

    receiver_.OnGuildInfo(updateTick_, guild);
}

void ProtocolGame::ParseGuildMemberList(InputMessage& message)
{
    std::vector<std::string> members;
    size_t count = message.Get<uint16_t>();
    members.reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
        members.push_back(message.GetString());
    }

    receiver_.OnGuildMemberList(updateTick_, members);
}

void ProtocolGame::ParsePlayerInfo(InputMessage& message)
{
    uint32_t fields = message.Get<uint32_t>();
    RelatedAccount frnd;
    frnd.fields = fields;
    frnd.accountUuid = message.GetString();
    if (fields & AB::GameProtocol::PlayerInfoFieldName)
        frnd.nickName = message.GetString();
    if (fields & AB::GameProtocol::PlayerInfoFieldRelation)
        frnd.relation = static_cast<RelatedAccount::Relation>(message.Get<uint8_t>());
    if (fields & AB::GameProtocol::PlayerInfoFieldOnlineStatus)
        frnd.status = static_cast<RelatedAccount::Status>(message.Get<uint8_t>());
    if (fields & AB::GameProtocol::PlayerInfoFieldCurrentName)
        frnd.currentName = message.GetString();
    if (fields & AB::GameProtocol::PlayerInfoFieldCurrentMap)
        frnd.currentMap = message.GetString();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildGuid)
        frnd.guildUuid = message.GetString();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildRole)
        frnd.guildRole = static_cast<RelatedAccount::GuildRole>(message.Get<uint8_t>());
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildInviteName)
        frnd.guildInviteName = message.GetString();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildInvited)
        frnd.invited = message.Get<int64_t>();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildJoined)
        frnd.joined = message.Get<int64_t>();
    if (fields & AB::GameProtocol::PlayerInfoFieldGuildExpires)
        frnd.expires = message.Get<int64_t>();

    receiver_.OnPlayerInfo(updateTick_, frnd);
}

void ProtocolGame::ParseObjectPosUpdate(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();
    Vec3 newPos
    {
        message.Get<float>(),
        message.Get<float>(),
        message.Get<float>()
    };

    receiver_.OnObjectPos(updateTick_, objectId, newPos);
}

void ProtocolGame::ParseLeaveObject(InputMessage& message)
{
    uint32_t objectId = message.Get<uint32_t>();

    receiver_.OnDespawnObject(updateTick_, objectId);
}

void ProtocolGame::ParseObjectSpawn(bool existing, InputMessage& message)
{
    using namespace AB::GameProtocol;

    ObjectSpawn os{ };
    os.id = message.Get<uint32_t>();
    os.type = static_cast<AB::GameProtocol::GameObjectType>(message.Get<uint8_t>());
    os.validFields = message.Get<uint32_t>();

    if (os.validFields & ObjectSpawnFieldPos)
    {
        os.pos = {
            message.Get<float>(),
            message.Get<float>(),
            message.Get<float>()
        };
    }
    if (os.validFields & ObjectSpawnFieldRot)
        os.rot = message.Get<float>();
    if (os.validFields & ObjectSpawnFieldScale)
    {
        os.scale = {
            message.Get<float>(),
            message.Get<float>(),
            message.Get<float>()
        };
    }
    if (os.validFields & ObjectSpawnFieldUndestroyable)
        os.undestroyable = message.Get<uint8_t>() != 0;
    if (os.validFields & ObjectSpawnFieldSelectable)
        os.selectable = message.Get<uint8_t>() != 0;
    if (os.validFields & ObjectSpawnFieldState)
        os.state = static_cast<AB::GameProtocol::CreatureState>(message.Get<uint8_t>());
    if (os.validFields & ObjectSpawnFieldSpeed)
        os.speed = message.Get<float>();
    if (os.validFields & ObjectSpawnFieldGroupId)
        os.groupId = message.Get<uint32_t>();
    if (os.validFields & ObjectSpawnFieldGroupPos)
        os.groupPos = message.Get<uint8_t>();

    std::string data = message.GetString();
    PropReadStream stream;
    stream.Init(data.c_str(), data.length());

    receiver_.OnObjectSpawn(updateTick_, os, stream, existing);
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

void ProtocolGame::ParsePlayerAutoRun(InputMessage& message)
{
    bool autorun = message.Get<uint8_t>() == 1;

    receiver_.OnPlayerAutorun(updateTick_, autorun);
}

void ProtocolGame::ParseServerJoined(InputMessage& message)
{
    AB::Entities::Service s;
    s.type = message.Get<AB::Entities::ServiceType>();
    s.uuid = message.GetString();
    s.host = message.GetString();
    s.port = message.Get<uint16_t>();
    s.location = message.GetString();
    s.name = message.GetString();

    receiver_.OnServerJoined(s);
}

void ProtocolGame::ParseServerLeft(InputMessage& message)
{
    AB::Entities::Service s;
    s.type = message.Get<AB::Entities::ServiceType>();
    s.uuid = message.GetString();
    s.host = message.GetString();
    s.port = message.Get<uint16_t>();
    s.location = message.GetString();
    s.name = message.GetString();

    receiver_.OnServerLeft(s);
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

void ProtocolGame::ParseEnterWorld(InputMessage& message)
{
    std::string serverId = message.GetString();
    std::string mapUuid = message.GetString();
    std::string instanceUuid = message.GetString();
    uint32_t playerId = message.Get<uint32_t>();
    AB::Entities::GameType type = static_cast<AB::Entities::GameType>(message.Get<uint8_t>());
    uint8_t partySize = message.Get<uint8_t>();

    receiver_.OnEnterWorld(updateTick_, serverId, mapUuid, instanceUuid, playerId, type, partySize);
}

void ProtocolGame::ParseChangeInstance(InputMessage& message)
{
    // The server is telling us to change the instance.
    // We should reply with an EnterWorld message.
    std::string serverId = message.GetString();
    std::string mapUuid = message.GetString();
    std::string instanceUuid = message.GetString();
    std::string charUuid = message.GetString();

    receiver_.OnChangeInstance(updateTick_, serverId, mapUuid, instanceUuid, charUuid);
}

void ProtocolGame::ParseMailHeaders(InputMessage& message)
{
    uint16_t mailCount = message.Get<uint16_t>();
    std::vector<AB::Entities::MailHeader> mailHeaders;
    for (uint16_t i = 0; i < mailCount; i++)
    {
        mailHeaders.push_back({
            message.GetString(),         // UUID
            message.GetString(),         // From name
            message.GetString(),         // Subject
            message.Get<int64_t>(),      // Created
            message.Get<uint8_t>() != 0  // Read
        });
    }
    // Sort Date Desc
    std::sort(mailHeaders.begin(), mailHeaders.end(), [](const auto& lhs, const auto& rhs)
    {
        return lhs.created - rhs.created;
    });

    receiver_.OnGetMailHeaders(updateTick_, mailHeaders);
}

void ProtocolGame::ParseMailComplete(InputMessage& message)
{
    AB::Entities::Mail mail;
    mail.fromAccountUuid = message.GetString();
    mail.fromName = message.GetString();
    mail.toName = message.GetString();
    mail.subject = message.GetString();
    mail.message = message.GetString();
    mail.created = message.Get<int64_t>();
    mail.isRead = message.Get<uint8_t>() != 0;

    receiver_.OnGetMail(updateTick_, mail);
}

void ProtocolGame::ParseInventoryContent(InputMessage& message)
{
    uint16_t count = message.Get<uint16_t>();
    std::vector<InventoryItem> items;
    items.reserve(count);
    for (uint16_t i = 0; i < count; i++)
    {
        items.push_back({
            static_cast<AB::Entities::ItemType>(message.Get<uint16_t>()),
            message.Get<uint32_t>(),
            static_cast<AB::Entities::StoragePlace>(message.Get<uint8_t>()),
            message.Get<uint16_t>(),
            message.Get<uint32_t>(),
            message.Get<uint16_t>()
        });
    }

    receiver_.OnGetInventory(updateTick_, items);
}

void ProtocolGame::ParseInventoryItemUpdate(InputMessage& message)
{
    InventoryItem item;
    item.type = static_cast<AB::Entities::ItemType>(message.Get<uint16_t>());
    item.index = message.Get<uint32_t>();
    item.place = static_cast<AB::Entities::StoragePlace>(message.Get<uint8_t>());
    item.pos = message.Get<uint16_t>();
    item.count = message.Get<uint32_t>();
    item.value = message.Get<uint16_t>();

    receiver_.OnInventoryItemUpdate(updateTick_, item);
}

void ProtocolGame::ParseInventoryItemDelete(InputMessage& message)
{
    uint16_t pos = message.Get<uint16_t>();
    receiver_.OnInventoryItemDelete(updateTick_, pos);
}

void ProtocolGame::ParseChestContent(InputMessage& message)
{
    uint16_t count = message.Get<uint16_t>();
    std::vector<InventoryItem> items;
    items.reserve(count);
    for (uint16_t i = 0; i < count; ++i)
    {
        items.push_back({
            static_cast<AB::Entities::ItemType>(message.Get<uint16_t>()),
            message.Get<uint32_t>(),
            static_cast<AB::Entities::StoragePlace>(message.Get<uint8_t>()),
            message.Get<uint16_t>(),
            message.Get<uint32_t>(),
            message.Get<uint16_t>()
        });
    }

    receiver_.OnGetChest(updateTick_, items);
}

void ProtocolGame::ParseChestItemUpdate(InputMessage& message)
{
    InventoryItem item;
    item.type = static_cast<AB::Entities::ItemType>(message.Get<uint16_t>());
    item.index = message.Get<uint32_t>();
    item.place = static_cast<AB::Entities::StoragePlace>(message.Get<uint8_t>());
    item.pos = message.Get<uint16_t>();
    item.count = message.Get<uint32_t>();
    item.value = message.Get<uint16_t>();

    receiver_.OnChestItemUpdate(updateTick_, item);
}

void ProtocolGame::ParseChestItemDelete(InputMessage& message)
{
    uint16_t pos = message.Get<uint16_t>();
    receiver_.OnChestItemDelete(updateTick_, pos);
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
    msg->Add<uint8_t>(static_cast<uint8_t>(ping ? 1 : 0));
    Send(std::move(msg));
}

void ProtocolGame::UseSkill(uint32_t index, bool ping)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeUseSkill);
    msg->Add<uint8_t>(static_cast<uint8_t>(index));
    msg->Add<uint8_t>(static_cast<uint8_t>(ping ? 1 : 0));
    Send(std::move(msg));
}

void ProtocolGame::Attack(bool ping)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeAttack);
    msg->Add<uint8_t>(static_cast<uint8_t>(ping ? 1 : 0));
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
