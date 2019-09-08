#include "stdafx.h"
#include "ProtocolGame.h"
#include "TimeUtils.h"
#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>

#include "DebugNew.h"

namespace Client {

ProtocolGame::ProtocolGame(Crypto::DHKeys& keys, asio::io_service& ioService) :
    Protocol(keys, ioService),
    updateTick_(0),
    loggingOut_(false)
{
    checksumEnabled_ = ProtocolGame::UseChecksum;
    compressionEnabled_ = ENABLE_GAME_COMPRESSION;
    encryptEnabled_ = ENABLE_GAME_ENCRYTION;
    SetEncKey(AB::ENC_KEY);
}

void ProtocolGame::Login(const std::string& accountUuid,
    const std::string& accountPass, const std::string& charUuid,
    const std::string& mapUuid,
    const std::string& instanceUuid,
    const std::string& host, uint16_t port)
{
    accountUuid_ = accountUuid;
    accountPass_ = accountPass;
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

void ProtocolGame::OnReceive(const std::shared_ptr<InputMessage>& message)
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
    if (receiver_)
        receiver_->OnNetworkError(connectionError, err);
}

void ProtocolGame::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    AB::GameProtocol::GameProtocolCodes opCode = AB::GameProtocol::NoError;
    while (!message->Eof())
    {
        opCode = static_cast<AB::GameProtocol::GameProtocolCodes>(message->Get<uint8_t>());

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
            ParseSpawnObject(opCode == AB::GameProtocol::GameSpawnObjectExisting, message);
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
        case AB::GameProtocol::FriendListAll:
            ParseFriendListAll(message);
            break;
        case AB::GameProtocol::GuildMembersAll:
            ParseGuildMembersAll(message);
            break;
        default:
            // End of message. Encryption adds some padding bytes, so after this
            // its probably just junk.
            return;
        }
    }
}

void ProtocolGame::ParseKeyExchange(const std::shared_ptr<InputMessage>& message)
{
    for (int i = 0; i < DH_KEY_LENGTH; ++i)
        serverKey_[i] = message->Get<uint8_t>();
}

void ProtocolGame::ParseObjectRotUpdate(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    float rot = message->Get<float>();
    bool manual = message->Get<uint8_t>() != 0;
    if (receiver_)
        receiver_->OnObjectRot(updateTick_, objectId, rot, manual);
}

void ProtocolGame::ParseObjectStateChange(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    AB::GameProtocol::CreatureState state = static_cast<AB::GameProtocol::CreatureState>(message->Get<uint8_t>());
    if (receiver_)
        receiver_->OnObjectStateChange(updateTick_, objectId, state);
}

void ProtocolGame::ParseObjectSpeedChange(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    float speedFactor = message->Get<float>();
    if (receiver_)
        receiver_->OnObjectSpeedChange(updateTick_, objectId, speedFactor);
}

void ProtocolGame::ParseObjectSelected(const std::shared_ptr<InputMessage>& message)
{
    uint32_t sourceId = message->Get<uint32_t>();
    uint32_t targetId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnObjectSelected(updateTick_, sourceId, targetId);
}

void ProtocolGame::ParseObjectSkillFailure(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    int skillIndex = static_cast<int>(message->Get<int8_t>());
    AB::GameProtocol::SkillError skillError = static_cast<AB::GameProtocol::SkillError>(message->Get<uint8_t>());
    if (receiver_)
        receiver_->OnObjectSkillFailure(updateTick_, objectId, skillIndex, skillError);
}

void ProtocolGame::ParseObjectUseSkill(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    // Index in the skill bar, 1 based
    int skillIndex = static_cast<int>(message->Get<uint8_t>());
    uint16_t energy = message->Get<uint16_t>();
    uint16_t adrenaline = message->Get<uint16_t>();
    uint16_t activation = message->Get<uint16_t>();
    uint16_t overcast = message->Get<uint16_t>();
    uint16_t hpScarifies = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnObjectUseSkill(updateTick_, objectId, skillIndex, energy, adrenaline, activation, overcast, hpScarifies);
}

void ProtocolGame::ParseObjectEndUseSkill(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    int skillIndex = static_cast<int>(message->Get<uint8_t>());
    uint16_t recharge = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnObjectEndUseSkill(updateTick_, objectId, skillIndex, recharge);
}

void ProtocolGame::ParseObjectAttackFailure(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    AB::GameProtocol::AttackError attackError = static_cast<AB::GameProtocol::AttackError>(message->Get<uint8_t>());
    if (receiver_)
        receiver_->OnObjectAttackFailure(updateTick_, objectId, attackError);
}

void ProtocolGame::ParseObjectPingTarget(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    uint32_t targetId = message->Get<uint32_t>();
    AB::GameProtocol::ObjectCallType type = static_cast<AB::GameProtocol::ObjectCallType>(message->Get<uint8_t>());
    int skillIndex = message->Get<int8_t>();
    if (receiver_)
        receiver_->OnObjectPingTarget(updateTick_, objectId, targetId, type, skillIndex);
}

void ProtocolGame::ParseObjectEffectAdded(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    uint32_t effectIndex = message->Get<uint32_t>();
    uint32_t ticks = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnObjectEffectAdded(updateTick_, objectId, effectIndex, ticks);
}

void ProtocolGame::ParseObjectEffectRemoved(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    uint32_t effectIndex = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnObjectEffectRemoved(updateTick_, objectId, effectIndex);
}

void ProtocolGame::ParseObjectDamaged(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    uint32_t sourceId = message->Get<uint32_t>();
    uint16_t index = message->Get<uint16_t>();
    uint8_t damageType = message->Get<uint8_t>();
    int16_t damageValue = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnObjectDamaged(updateTick_, objectId, sourceId, index, damageType, damageValue);
}

void ProtocolGame::ParseObjectHealed(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    uint32_t healerId = message->Get<uint32_t>();
    uint16_t index = message->Get<uint16_t>();
    int16_t healValue = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnObjectHealed(updateTick_, objectId, healerId, index, healValue);
}

void ProtocolGame::ParseObjectProgress(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    AB::GameProtocol::ObjectProgressType type = static_cast<AB::GameProtocol::ObjectProgressType>(message->Get<uint8_t>());
    int value = static_cast<int>(message->Get<int16_t>());
    if (receiver_)
        receiver_->OnObjectProgress(updateTick_, objectId, type, value);
}

void ProtocolGame::ParseObjectDroppedItem(const std::shared_ptr<InputMessage>& message)
{
    uint32_t dropperId = message->Get<uint32_t>();
    uint32_t targetId = message->Get<uint32_t>();
    uint32_t itemId = message->Get<uint32_t>();
    uint32_t itemIndex = message->Get<uint32_t>();
    uint32_t count = message->Get<uint32_t>();
    uint16_t value = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnObjectDroppedItem(updateTick_, dropperId, targetId, itemId, itemIndex, count, value);
}

void ProtocolGame::ParseObjectSetPosition(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    Vec3 pos = {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    if (receiver_)
        receiver_->OnObjectSetPosition(updateTick_, objectId, pos);
}

void ProtocolGame::ParseServerMessage(const std::shared_ptr<InputMessage>& message)
{
    AB::GameProtocol::ServerMessageType type =
        static_cast<AB::GameProtocol::ServerMessageType>(message->Get<uint8_t>());
    std::string sender = message->GetString();
    std::string data = message->GetString();
    if (receiver_)
        receiver_->OnServerMessage(updateTick_, type, sender, data);
}

void ProtocolGame::ParseChatMessage(const std::shared_ptr<InputMessage>& message)
{
    AB::GameProtocol::ChatMessageChannel type =
        static_cast<AB::GameProtocol::ChatMessageChannel>(message->Get<uint8_t>());
    uint32_t senderId = message->Get<uint32_t>();
    std::string sender = message->GetString();
    std::string data = message->GetString();
    if (receiver_)
        receiver_->OnChatMessage(updateTick_, type, senderId, sender, data);
}

void ProtocolGame::ParsePartyPlayerInvited(const std::shared_ptr<InputMessage>& message)
{
    uint32_t sourceId = message->Get<uint32_t>();         // Inviter (source)
    uint32_t targetId = message->Get<uint32_t>();         // Invitee (target)
    uint32_t partyId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnPartyInvited(updateTick_, sourceId, targetId, partyId);
}

void ProtocolGame::ParsePartyPlayerRemoved(const std::shared_ptr<InputMessage>& message)
{
    uint32_t sourceId = message->Get<uint32_t>();
    uint32_t targetId = message->Get<uint32_t>();
    uint32_t partyId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnPartyRemoved(updateTick_, sourceId, targetId, partyId);
}

void ProtocolGame::ParsePartyPlayerAdded(const std::shared_ptr<InputMessage>& message)
{
    uint32_t acceptorId = message->Get<uint32_t>();
    uint32_t leaderId = message->Get<uint32_t>();
    uint32_t partyId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnPartyAdded(updateTick_, acceptorId, leaderId, partyId);
}

void ProtocolGame::ParsePartyInviteRemoved(const std::shared_ptr<InputMessage>& message)
{
    uint32_t sourceId = message->Get<uint32_t>();
    uint32_t targetId = message->Get<uint32_t>();
    uint32_t partyId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnPartyInviteRemoved(updateTick_, sourceId, targetId, partyId);
}

void ProtocolGame::ParsePartyResigned(const std::shared_ptr<InputMessage>& message)
{
    uint32_t partyId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnPartyResigned(updateTick_, partyId);
}

void ProtocolGame::ParsePartyDefeated(const std::shared_ptr<InputMessage>& message)
{
    uint32_t partyId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnPartyDefeated(updateTick_, partyId);
}

void ProtocolGame::ParsePartyInfoMembers(const std::shared_ptr<InputMessage>& message)
{
    uint32_t partyId = message->Get<uint32_t>();
    size_t count = message->Get<uint8_t>();
    std::vector<uint32_t> members;
    members.resize(count);
    for (size_t i = 0; i < count; i++)
    {
        members[i] = message->Get<uint32_t>();
    }
    if (receiver_)
        receiver_->OnPartyInfoMembers(partyId, members);
}

void ProtocolGame::ParseResourceChanged(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    AB::GameProtocol::ResourceType resType = static_cast<AB::GameProtocol::ResourceType>(message->Get<uint8_t>());
    switch (resType)
    {
    case AB::GameProtocol::ResourceTypeHealth:
    case AB::GameProtocol::ResourceTypeEnergy:
    case AB::GameProtocol::ResourceTypeAdrenaline:
    case AB::GameProtocol::ResourceTypeOvercast:
    case AB::GameProtocol::ResourceTypeMaxHealth:
    case AB::GameProtocol::ResourceTypeMaxEnergy:
    {
        int16_t value = message->Get<int16_t>();
        if (receiver_)
            receiver_->OnResourceChanged(updateTick_, objectId, resType, value);
        break;
    }
    case AB::GameProtocol::ResourceTypeHealthRegen:
    case AB::GameProtocol::ResourceTypeEnergyRegen:
    {
        int8_t value = message->Get<int8_t>();
        if (receiver_)
            receiver_->OnResourceChanged(updateTick_, objectId, resType, static_cast<int16_t>(value));
        break;
    }
    }
}

void ProtocolGame::ParseDialogTrigger(const std::shared_ptr<InputMessage>& message)
{
    uint32_t dialogId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnDialogTrigger(updateTick_, dialogId);
}

void ProtocolGame::ParseFriendListAll(const std::shared_ptr<InputMessage>& message)
{
    std::vector<RelatedAccount> friends;
    size_t count = message->Get<uint16_t>();
    friends.reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
        friends.push_back({
            static_cast<RelatedAccount::Releation>(message->Get<uint8_t>()),
            message->GetString(),                         // Account UUID
            message->GetString(),                         // Name
            static_cast<RelatedAccount::Status>(message->Get<uint8_t>()),
            message->GetString(),                         // Current name
            message->GetString()                          // Current map
        });
    }
    // TODO:
    (void)friends;
}

void ProtocolGame::ParseGuildMembersAll(const std::shared_ptr<InputMessage>& message)
{
    // TODO:
    (void)message;
}

void ProtocolGame::ParseObjectPosUpdate(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    Vec3 newPos
    {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    if (receiver_)
        receiver_->OnObjectPos(updateTick_, objectId, newPos);
}

void ProtocolGame::ParseLeaveObject(const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnDespawnObject(updateTick_, objectId);
}

void ProtocolGame::ParseSpawnObject(bool existing, const std::shared_ptr<InputMessage>& message)
{
    uint32_t objectId = message->Get<uint32_t>();
    ObjectSpawn os;
    os.pos = {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    os.rot = message->Get<float>();
    os.scale = {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    os.undestroyable = message->Get<uint8_t>() != 0;
    os.selectable = message->Get<uint8_t>() != 0;
    os.state = static_cast<AB::GameProtocol::CreatureState>(message->Get<uint8_t>());
    os.speed = message->Get<float>();
    os.groupId = message->Get<uint32_t>();
    os.groupPos = message->Get<uint8_t>();

    std::string data = message->GetString();
    PropReadStream stream;
    stream.Init(data.c_str(), data.length());
    if (receiver_)
        receiver_->OnSpawnObject(updateTick_, objectId, os, stream, existing);
}

void ProtocolGame::ParseUpdate(const std::shared_ptr<InputMessage>& message)
{
    updateTick_ = message->Get<int64_t>();
}

void ProtocolGame::ParsePong(const std::shared_ptr<InputMessage>& message)
{
    int32_t diff = message->Get<int32_t>();
    // Clock difference between client and server
    clockDiff_ = static_cast<int64_t>(diff);
    // Round trip time
    lastPing_ = static_cast<int>(AbTick() - pingTick_);
    if (receiver_)
        receiver_->OnPong(lastPing_);
}

void ProtocolGame::ParseGameError(const std::shared_ptr<InputMessage>& message)
{
    AB::GameProtocol::PlayerErrorValue error = static_cast<AB::GameProtocol::PlayerErrorValue>(message->Get<uint8_t>());
    if (receiver_)
        receiver_->OnPlayerError(updateTick_, error);
}

void ProtocolGame::ParsePlayerAutoRun(const std::shared_ptr<InputMessage>& message)
{
    bool autorun = message->Get<uint8_t>() == 1;
    if (receiver_)
        receiver_->OnPlayerAutorun(updateTick_, autorun);
}

void ProtocolGame::ParseServerJoined(const std::shared_ptr<InputMessage>& message)
{
    AB::Entities::Service s;
    s.type = message->Get<AB::Entities::ServiceType>();
    s.uuid = message->GetString();
    s.host = message->GetString();
    s.port = message->Get<uint16_t>();
    s.location = message->GetString();
    s.name = message->GetString();
    if (receiver_)
        receiver_->OnServerJoined(s);
}

void ProtocolGame::ParseServerLeft(const std::shared_ptr<InputMessage>& message)
{
    AB::Entities::Service s;
    s.type = message->Get<AB::Entities::ServiceType>();
    s.uuid = message->GetString();
    s.host = message->GetString();
    s.port = message->Get<uint16_t>();
    s.location = message->GetString();
    s.name = message->GetString();
    if (receiver_)
        receiver_->OnServerLeft(s);
}

void ProtocolGame::ParseError(const std::shared_ptr<InputMessage>& message)
{
    uint8_t error = message->Get<uint8_t>();
    if (error != 0)
        ProtocolError(error);
}

void ProtocolGame::ParseEnterWorld(const std::shared_ptr<InputMessage>& message)
{
    std::string serverId = message->GetString();
    std::string mapUuid = message->GetString();
    std::string instanceUuid = message->GetString();
    uint32_t playerId = message->Get<uint32_t>();
    AB::Entities::GameType type = static_cast<AB::Entities::GameType>(message->Get<uint8_t>());
    uint8_t partySize = message->Get<uint8_t>();
    if (receiver_)
        receiver_->OnEnterWorld(updateTick_, serverId, mapUuid, instanceUuid, playerId, type, partySize);
}

void ProtocolGame::ParseChangeInstance(const std::shared_ptr<InputMessage>& message)
{
    // The server is telling us to change the instance.
    // We should reply with an EnterWorld message.
    std::string serverId = message->GetString();
    std::string mapUuid = message->GetString();
    std::string instanceUuid = message->GetString();
    std::string charUuid = message->GetString();
    if (receiver_)
        receiver_->OnChangeInstance(updateTick_, serverId, mapUuid, instanceUuid, charUuid);
}

void ProtocolGame::ParseMailHeaders(const std::shared_ptr<InputMessage>& message)
{
    uint16_t mailCount = message->Get<uint16_t>();
    std::vector<AB::Entities::MailHeader> mailHeaders;
    for (uint16_t i = 0; i < mailCount; i++)
    {
        mailHeaders.push_back({
            message->GetString(),         // UUID
            message->GetString(),         // From name
            message->GetString(),         // Subject
            message->Get<int64_t>(),      // Created
            message->Get<uint8_t>() != 0  // Read
        });
    }
    // Sort Date Desc
    std::sort(mailHeaders.begin(), mailHeaders.end(), [](const auto& lhs, const auto& rhs)
    {
        return lhs.created - rhs.created;
    });
    if (receiver_)
        receiver_->OnGetMailHeaders(updateTick_, mailHeaders);
}

void ProtocolGame::ParseMailComplete(const std::shared_ptr<InputMessage>& message)
{
    AB::Entities::Mail mail;
    mail.fromAccountUuid = message->GetString();
    mail.fromName = message->GetString();
    mail.toName = message->GetString();
    mail.subject = message->GetString();
    mail.message = message->GetString();
    mail.created = message->Get<int64_t>();
    mail.isRead = message->Get<uint8_t>() != 0;
    if (receiver_)
        receiver_->OnGetMail(updateTick_, mail);
}

void ProtocolGame::ParseInventoryContent(const std::shared_ptr<InputMessage>& message)
{
    uint16_t count = message->Get<uint16_t>();
    std::vector<InventoryItem> items;
    items.reserve(count);
    for (uint16_t i = 0; i < count; i++)
    {
        items.push_back({
            static_cast<AB::Entities::ItemType>(message->Get<uint16_t>()),
            message->Get<uint32_t>(),
            static_cast<AB::Entities::StoragePlace>(message->Get<uint8_t>()),
            message->Get<uint16_t>(),
            message->Get<uint32_t>(),
            message->Get<uint16_t>()
        });
    }
    if (receiver_)
        receiver_->OnGetInventory(updateTick_, items);
}

void ProtocolGame::ParseInventoryItemUpdate(const std::shared_ptr<InputMessage>& message)
{
    InventoryItem item;
    item.type = static_cast<AB::Entities::ItemType>(message->Get<uint16_t>());
    item.index = message->Get<uint32_t>();
    item.place = static_cast<AB::Entities::StoragePlace>(message->Get<uint8_t>());
    item.pos = message->Get<uint16_t>();
    item.count = message->Get<uint32_t>();
    item.value = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnInventoryItemUpdate(updateTick_, item);
}

void ProtocolGame::ParseInventoryItemDelete(const std::shared_ptr<InputMessage>& message)
{
    uint16_t pos = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnInventoryItemDelete(updateTick_, pos);
}

void ProtocolGame::ParseChestContent(const std::shared_ptr<InputMessage>& message)
{
    uint16_t count = message->Get<uint16_t>();
    std::vector<InventoryItem> items;
    items.reserve(count);
    for (uint16_t i = 0; i < count; ++i)
    {
        items.push_back({
            static_cast<AB::Entities::ItemType>(message->Get<uint16_t>()),
            message->Get<uint32_t>(),
            static_cast<AB::Entities::StoragePlace>(message->Get<uint8_t>()),
            message->Get<uint16_t>(),
            message->Get<uint32_t>(),
            message->Get<uint16_t>()
            });
    }
    if (receiver_)
        receiver_->OnGetChest(updateTick_, items);
}

void ProtocolGame::ParseChestItemUpdate(const std::shared_ptr<InputMessage>& message)
{
    InventoryItem item;
    item.type = static_cast<AB::Entities::ItemType>(message->Get<uint16_t>());
    item.index = message->Get<uint32_t>();
    item.place = static_cast<AB::Entities::StoragePlace>(message->Get<uint8_t>());
    item.pos = message->Get<uint16_t>();
    item.count = message->Get<uint32_t>();
    item.value = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnChestItemUpdate(updateTick_, item);
}

void ProtocolGame::ParseChestItemDelete(const std::shared_ptr<InputMessage>& message)
{
    uint16_t pos = message->Get<uint16_t>();
    if (receiver_)
        receiver_->OnChestItemDelete(updateTick_, pos);
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
    msg->AddString(accountPass_);
    msg->AddString(charUuid_);
    msg->AddString(mapUuid_);
    msg->AddString(instanceUuid_);
    Send(msg);
}

void ProtocolGame::Logout()
{
    loggingOut_ = true;
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeLogout);
    Send(msg);
}

void ProtocolGame::Ping()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePing);
    pingTick_ = AbTick();
    msg->Add<int64_t>(pingTick_),
    Send(msg);
}

void ProtocolGame::ChangeMap(const std::string& mapUuid)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeChangeMap);
    msg->AddString(mapUuid);
    Send(msg);
}

void ProtocolGame::GetMailHeaders()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetMailHeaders);
    Send(msg);
}

void ProtocolGame::GetInventory()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetInventory);
    Send(msg);
}

void ProtocolGame::InventoryStoreItem(uint16_t pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeInventoryStoreInChest);
    msg->Add<uint16_t>(pos);
    Send(msg);
}

void ProtocolGame::InventoryDestroyItem(uint16_t pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeInventoryDestroyItem);
    msg->Add<uint16_t>(pos);
    Send(msg);
}

void ProtocolGame::InventoryDropItem(uint16_t pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeInventoryDropItem);
    msg->Add<uint16_t>(pos);
    Send(msg);
}

void ProtocolGame::GetChest()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetChest);
    Send(msg);
}

void ProtocolGame::ChestDestroyItem(uint16_t pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeChestDestroyItem);
    msg->Add<uint16_t>(pos);
    Send(msg);
}

void ProtocolGame::GetMail(const std::string& mailUuid)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetMail);
    msg->AddString(mailUuid);
    Send(msg);
}

void ProtocolGame::DeleteMail(const std::string& mailUuid)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeDeleteMail);
    msg->AddString(mailUuid);
    Send(msg);
}

void ProtocolGame::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSendMail);
    msg->AddString(recipient);
    msg->AddString(subject);
    msg->AddString(body);
    Send(msg);
}

void ProtocolGame::Move(uint8_t direction)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeMove);
    msg->Add<uint8_t>(direction);
    Send(msg);
}

void ProtocolGame::Turn(uint8_t direction)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeTurn);
    msg->Add<uint8_t>(direction);
    Send(msg);
}

void ProtocolGame::SetDirection(float rad)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSetDirection);
    msg->Add<float>(rad);
    Send(msg);
}

void ProtocolGame::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeClickObject);
    msg->Add<uint32_t>(sourceId);
    msg->Add<uint32_t>(targetId);
    Send(msg);
}

void ProtocolGame::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSelect);
    msg->Add<uint32_t>(sourceId);
    msg->Add<uint32_t>(targetId);
    Send(msg);
}

void ProtocolGame::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeCommand);
    msg->Add<uint8_t>(type);
    msg->AddString(data);
    Send(msg);
}

void ProtocolGame::GotoPos(const Vec3& pos)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGoto);
    msg->Add<float>(pos.x);
    msg->Add<float>(pos.y);
    msg->Add<float>(pos.z);
    Send(msg);
}

void ProtocolGame::Follow(uint32_t targetId, bool ping)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeFollow);
    msg->Add<uint32_t>(targetId);
    msg->Add<uint8_t>(static_cast<uint8_t>(ping ? 1 : 0));
    Send(msg);
}

void ProtocolGame::UseSkill(uint32_t index, bool ping)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeUseSkill);
    msg->Add<uint8_t>(static_cast<uint8_t>(index));
    msg->Add<uint8_t>(static_cast<uint8_t>(ping ? 1 : 0));
    Send(msg);
}

void ProtocolGame::Attack(bool ping)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeAttack);
    msg->Add<uint8_t>(static_cast<uint8_t>(ping ? 1 : 0));
    Send(msg);
}

void ProtocolGame::Cancel()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeCancel);
    Send(msg);
}

void ProtocolGame::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSetState);
    msg->Add<uint8_t>(static_cast<uint8_t>(newState));
    Send(msg);
}

void ProtocolGame::PartyInvitePlayer(uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyInvitePlayer);
    msg->Add<uint32_t>(targetId);
    Send(msg);
}

void ProtocolGame::PartyKickPlayer(uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyKickPlayer);
    msg->Add<uint32_t>(targetId);
    Send(msg);
}

void ProtocolGame::PartyLeave()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyLeave);
    Send(msg);
}

void ProtocolGame::PartyAcceptInvite(uint32_t inviterId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyAcceptInvite);
    msg->Add<uint32_t>(inviterId);
    Send(msg);
}

void ProtocolGame::PartyRejectInvite(uint32_t inviterId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePartyRejectInvite);
    msg->Add<uint32_t>(inviterId);
    Send(msg);
}

void ProtocolGame::PartyGetMembers(uint32_t partyId)
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacektTypeGetPartyMembers);
    msg->Add<uint32_t>(partyId);
    Send(msg);
}

void ProtocolGame::QueueMatch()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeQueue);
    Send(msg);
}

void ProtocolGame::UnqueueMatch()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeUnqueue);
    Send(msg);
}

}
