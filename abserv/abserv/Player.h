#pragma once

#include "ProtocolGame.h"
#include "GameObject.h"
#include "Game.h"
#include "Effect.h"
#include "Actor.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Account.h>
#include "FriendList.h"
#include "Party.h"

namespace Game {

class PlayerManager;
class MailBox;

class Player final : public Actor
{
private:
    std::unique_ptr<MailBox> mailBox_;
    std::unique_ptr<FriendList> friendList_;
    std::shared_ptr<Party> party_;
protected:
    friend class PlayerManager;
    inline std::shared_ptr<Player> GetThis()
    {
        return std::static_pointer_cast<Player>(shared_from_this());
    }
    void HandleCommand(AB::GameProtocol::CommandTypes type,
        const std::string& command, Net::NetworkMessage& message) override;
    void HandleServerIdCommand(const std::string&, Net::NetworkMessage&);
    void HandleWhisperCommand(const std::string& command, Net::NetworkMessage& message);
    void HandleChatGuildCommand(const std::string& command, Net::NetworkMessage&);
    void HandleChatTradeCommand(const std::string& command, Net::NetworkMessage&);
    void HandleAgeCommand(const std::string& command, Net::NetworkMessage& message);
    void HandleRollCommand(const std::string& command, Net::NetworkMessage& message);
    void HandleSitCommand(const std::string&, Net::NetworkMessage&);
    void HandleStandCommand(const std::string&, Net::NetworkMessage&);
    void HandleCryCommand(const std::string&, Net::NetworkMessage&);
    void HandleTauntCommand(const std::string&, Net::NetworkMessage&);
    void HandlePonderCommand(const std::string&, Net::NetworkMessage&);
    void HandleWaveCommand(const std::string&, Net::NetworkMessage&);
    void HandleLaughCommand(const std::string&, Net::NetworkMessage&);
    void HandleDieCommand(const std::string&, Net::NetworkMessage&);
    void HandleGeneralChatCommand(const std::string& command, Net::NetworkMessage&);
    void HandlePartyChatCommand(const std::string& command, Net::NetworkMessage&);
public:
    static void RegisterLua(kaguya::State& state);

    explicit Player(std::shared_ptr<Net::ProtocolGame> client);
    ~Player() override;
    // non-copyable
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    /// We are entering a game
    void SetGame(std::shared_ptr<Game> game) final override;
    std::string GetName() const final override { return data_.name; }
    AB::Entities::CharacterSex GetSex() const final override
    {
        return data_.sex;
    }
    uint32_t GetModelIndex() const final override
    {
        return data_.modelIndex;
    }
    uint32_t GetGroupId() const final override
    {
        return party_->id_;
    }
    uint8_t GetGroupPos() final override;

    uint32_t GetLevel() const final override { return data_.level; }
    AB::GameProtocol::GameObjectType GetType() const final override
    {
        return AB::GameProtocol::ObjectTypePlayer;
    }
    uint32_t GetInactiveTime() const
    {
        if (lastPing_ == 0)
            return 0;
        return static_cast<uint32_t>(Utils::AbTick() - lastPing_);
    }

    void Initialize();
    void Logout();
    void Ping();
    void ChangeMap(const std::string mapUuid);
    void ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid);
    void UpdateMailBox();
    void GetMailHeaders();
    void SendMail(const std::string recipient, const std::string subject, const std::string body);
    void GetMail(const std::string mailUuid);
    void DeleteMail(const std::string mailUuid);
    void NotifyNewMail();

    void SetParty(std::shared_ptr<Party> party);
    std::shared_ptr<Party> GetParty() const
    {
        return party_;
    }

    void PartyInvitePlayer(uint32_t playerId);
    void PartyKickPlayer(uint32_t playerId);
    /// Leave current party
    void PartyLeave();
    void PartyAccept(uint32_t inviterId);
    /// We reject the invite of inviterId
    void PartyRejectInvite(uint32_t inviterId);
    void PartyGetMembers(uint32_t partyId);

    AB::Entities::Character data_;
    AB::Entities::Account account_;
    time_t loginTime_;
    time_t logoutTime_;
    int64_t lastPing_ = 0;
    std::shared_ptr<Net::ProtocolGame> client_;

};

}
