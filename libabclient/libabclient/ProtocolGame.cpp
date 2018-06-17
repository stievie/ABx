#include "stdafx.h"
#include "ProtocolGame.h"
#include "TimeUtils.h"
#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>

#include "DebugNew.h"

namespace Client {

ProtocolGame::ProtocolGame() :
    Protocol(),
    updateTick_(0)
{
    checksumEnabled_ = ProtocolGame::UseChecksum;
    encryptEnabled_ = ENABLE_GAME_ENCRYTION;
}

void ProtocolGame::Login(const std::string& accountUuid,
    const std::string& accountPass, const std::string& charUuid,
    const std::string& mapUuid,
    const std::string& host, uint16_t port)
{
    accountUuid_ = accountUuid;
    accountPass_ = accountPass;
    charUuid_ = charUuid;
    mapUuid_ = mapUuid;

    Connect(host, port);
}

void ProtocolGame::OnConnect()
{
    firstRevc_ = true;
    Protocol::OnConnect();

    SendLoginPacket();

    Receive();
}

void ProtocolGame::OnReceive(const std::shared_ptr<InputMessage>& message)
{
    if (firstRevc_)
    {
        firstRevc_ = false;
    }
    ParseMessage(message);
    Receive();
}

void ProtocolGame::OnError(const asio::error_code& err)
{
    Protocol::OnError(err);
}

void ProtocolGame::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    AB::GameProtocol::GameProtocolCodes opCode = AB::GameProtocol::NoError;
    while (!message->Eof())
    {
        opCode = static_cast<AB::GameProtocol::GameProtocolCodes>(message->Get<uint8_t>());

        switch (opCode)
        {
        case AB::GameProtocol::Error:
            ParseError(message);
            break;
        case AB::GameProtocol::GameEnter:
            ParseEnterWorld(message);
            break;
        case AB::GameProtocol::GamePong:
            ParsePong(message);
            break;
        case AB::GameProtocol::MailHeaders:
            ParseMailHeaders(message);
            break;
        case AB::GameProtocol::MailComplete:
            ParseMailComplete(message);
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
        case AB::GameProtocol::GameObjectSelectTarget:
            ParseObjectSelected(message);
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
        default:
            // End of message. Encryption adds some padding bytes, so after this
            // its probably just junk.
            return;
        }
    }
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

void ProtocolGame::ParseObjectSelected(const std::shared_ptr<InputMessage>& message)
{
    uint32_t sourceId = message->Get<uint32_t>();
    uint32_t targetId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnObjectSelected(updateTick_, sourceId, targetId);
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
    uint32_t id = message->Get<uint32_t>();
}

void ProtocolGame::ParsePartyPlayerRemoved(const std::shared_ptr<InputMessage>& message)
{
    uint32_t id = message->Get<uint32_t>();
}

void ProtocolGame::ParsePartyPlayerAdded(const std::shared_ptr<InputMessage>& message)
{
    uint32_t id = message->Get<uint32_t>();
}

void ProtocolGame::ParsePartyInviteRemoved(const std::shared_ptr<InputMessage>& message)
{
    uint32_t id = message->Get<uint32_t>();
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

    Vec3 pos
    {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    float rot = message->Get<float>();
    Vec3 scale
    {
        message->Get<float>(),
        message->Get<float>(),
        message->Get<float>()
    };
    std::string data = message->GetString();
    PropReadStream stream;
    stream.Init(data.c_str(), data.length());
    if (receiver_)
        receiver_->OnSpawnObject(updateTick_, objectId, pos, scale, rot, stream, existing);
}

void ProtocolGame::ParseUpdate(const std::shared_ptr<InputMessage>& message)
{
    updateTick_ = message->Get<int64_t>();
    clockDiff_ = AbTick() - updateTick_;
}

void ProtocolGame::ParsePong(const std::shared_ptr<InputMessage>& message)
{
    AB_UNUSED(message);
    lastPing_ = static_cast<int>(AbTick() - pingTick_);
    if (receiver_)
        receiver_->OnPong(lastPing_);
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
    uint32_t playerId = message->Get<uint32_t>();
    if (receiver_)
        receiver_->OnEnterWorld(updateTick_, serverId, mapUuid, playerId);
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

void ProtocolGame::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolGame::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->AddString(accountUuid_);
    msg->AddString(accountPass_);
    msg->AddString(charUuid_);
    msg->AddString(mapUuid_);
    Send(msg);
}

void ProtocolGame::Logout()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeLogout);
    Send(msg);
    Connection::Poll();
}

void ProtocolGame::Ping()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypePing);
    pingTick_ = AbTick();
    Send(msg);
}

void ProtocolGame::GetMailHeaders()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetMailHeaders);
    Send(msg);
}

void ProtocolGame::GetMail(const std::string& mailUuid)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGetMail);
    msg->AddString(mailUuid);
    Send(msg);
}

void ProtocolGame::DeleteMail(const std::string& mailUuid)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeDeleteMail);
    msg->AddString(mailUuid);
    Send(msg);
}

void ProtocolGame::Move(uint8_t direction)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeMove);
    msg->Add<uint8_t>(direction);
    Send(msg);
}

void ProtocolGame::Turn(uint8_t direction)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeTurn);
    msg->Add<uint8_t>(direction);
    Send(msg);
}

void ProtocolGame::SetDirection(float rad)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSetDirection);
    msg->Add<float>(rad);
    Send(msg);
}

void ProtocolGame::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeSelect);
    msg->Add<uint32_t>(sourceId);
    msg->Add<uint32_t>(targetId);
    Send(msg);
}

void ProtocolGame::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeCommand);
    msg->Add<uint8_t>(type);
    msg->AddString(data);
    Send(msg);
}

void ProtocolGame::GotoPos(const Vec3& pos)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeGoto);
    msg->Add<float>(pos.x);
    msg->Add<float>(pos.y);
    msg->Add<float>(pos.z);
    Send(msg);
}

void ProtocolGame::Follow(uint32_t targetId)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AB::GameProtocol::PacketTypeFollow);
    msg->Add<uint32_t>(targetId);
    Send(msg);
}

}
