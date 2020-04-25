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

#pragma once

#include "Protocol.h"
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
protected:
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
        msg.Add<uint8_t>(static_cast<uint8_t>(type));
        AB::Packets::Add(packet, msg);
        Send(msg);
    }
    /// Triggers OnPong()
    void Ping();
    void ChangeMap(const std::string& mapUuid);
    void GetMailHeaders();
    void GetInventory();
    void InventoryDestroyItem(uint16_t pos);
    void InventoryDropItem(uint16_t pos);
    void SetItemPos(AB::Entities::StoragePlace currentPlace, uint16_t currentPos,
        AB::Entities::StoragePlace place, uint16_t newPos);
    void GetChest();
    void ChestDestroyItem(uint16_t pos);
    void DepositMoney(uint32_t amount);
    void WithdrawMoney(uint32_t amount);
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
    void Command(AB::GameProtocol::CommandType type, const std::string& data);
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
    void SetSecondaryProfession(uint32_t profIndex);
    void SetAttributeValue(uint32_t attribIndex, uint8_t value);
    void EquipSkill(uint32_t skillIndex, uint8_t pos);
    void LoadSkillTemplate(const std::string& templ);
    void TradeRequest(uint32_t targetId);
    void TradeCancel();
    void TradeOffer(uint32_t money, std::vector<std::pair<uint16_t, uint32_t>>&& items);
    void TradeAccept();

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
