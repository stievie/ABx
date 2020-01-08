#pragma once

#include "Protocol.h"
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/FriendList.h>
#include "Receiver.h"
#include <AB/Packets/ServerPackets.h>
#include <AB/Packets/Packet.h>
#include <sa/CallableTable.h>

namespace Client {

class ProtocolGame : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
private:
    Receiver& receiver_;
    std::string accountUuid_;
    std::string authToken_;
    std::string charUuid_;
    std::string mapUuid_;
    std::string instanceUuid_;
    int64_t pingTick_;
    int64_t updateTick_;
    int64_t clockDiff_;
    int lastPing_;
    bool firstRevc_;
    DH_KEY serverKey_;
    bool loggingOut_;

    // Lookup table code -> packet
    sa::CallableTable<AB::GameProtocol::ServerPacketType, void, InputMessage&> packetHandlers_;
    template<typename P, AB::GameProtocol::ServerPacketType _Type>
    void AddHandler()
    {
        packetHandlers_.Add(_Type, [this](InputMessage& message)
        {
            auto packet = AB::Packets::Get<P>(message);
            receiver_.OnPacket(updateTick_, packet);
        });
    }

    void LogMessage(const std::string& message);
    void SendLoginPacket();
protected:
    void OnConnect() override;
    void OnReceive(InputMessage& message) override;
    void OnError(ConnectionError connectionError, const asio::error_code& err) override;

    void ParseMessage(InputMessage& message);
    void ParseKeyExchange(InputMessage& message);
public:
    ProtocolGame(Receiver& receiver, Crypto::DHKeys& keys, asio::io_service& ioService);
    ~ProtocolGame() override = default;

    void Login(const std::string& accountUuid, const std::string& authToken,
        const std::string& charUuid, const std::string& mapUuid, const std::string& instanceUuid,
        const std::string& host, uint16_t port);
    void Logout();

    template <typename T>
    void SendPacket(AB::GameProtocol::ClientPacketTypes type, T& packet)
    {
        OutputMessage msg;
        msg.Add<uint8_t>(type);
        AB::Packets::Add(packet, msg);
        Send(msg);
    }
    /// Triggers OnPong()
    void Ping();
    void ChangeMap(const std::string& mapUuid);
    void GetMailHeaders();
    void GetInventory();
    void InventoryStoreItem(uint16_t pos);
    void InventoryDestroyItem(uint16_t pos);
    void InventoryDropItem(uint16_t pos);
    void GetChest();
    void ChestDestroyItem(uint16_t pos);
    void GetMail(const std::string& mailUuid);
    void DeleteMail(const std::string& mailUuid);
    void SendMail(const std::string& recipient, const std::string& subject, const std::string& body);
    void GetPlayerInfoByName(const std::string& name, uint32_t fields);
    void GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields);

    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void Command(AB::GameProtocol::CommandTypes type, const std::string& data);
    void GotoPos(const Vec3& pos);
    void Follow(uint32_t targetId, bool ping);
    void UseSkill(uint32_t index, bool ping);
    void Attack(bool ping);
    void Cancel();
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
    void PartyInvitePlayer(uint32_t targetId);
    void PartyKickPlayer(uint32_t targetId);
    void PartyLeave();
    void PartyAcceptInvite(uint32_t inviterId);
    void PartyRejectInvite(uint32_t inviterId);
    void PartyGetMembers(uint32_t partyId);
    void QueueMatch();
    void UnqueueMatch();
    void AddFriend(const std::string& name, AB::Entities::FriendRelation relation);
    void RemoveFriend(const std::string& accountUuid);
    void UpdateFriendList();
    void SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status);

    int64_t GetUpdateTick() const
    {
        return updateTick_;
    }
    int64_t GetClockDiff() const
    {
        return clockDiff_;
    }
};

}
