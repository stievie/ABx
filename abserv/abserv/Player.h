#pragma once

#include "ProtocolGame.h"
#include "GameObject.h"
#include "Game.h"
#include "Effect.h"
#include "Creature.h"
#include <AB/Entities/Character.h>

namespace Game {

class PlayerManager;
class MailBox;

class Player final : public Creature
{
private:
    std::unique_ptr<MailBox> mailBox_;
protected:
    friend class PlayerManager;
    std::shared_ptr<Player> GetThis()
    {
        return std::static_pointer_cast<Player>(shared_from_this());
    }
    void HandleCommand(AB::GameProtocol::CommandTypes type,
        const std::string& command, Net::NetworkMessage& message) override;
public:
    static void RegisterLua(kaguya::State& state);

    explicit Player(std::shared_ptr<Net::ProtocolGame> client);
    ~Player() override;
    // non-copyable
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    std::string GetName() const override { return data_.name; }
    uint32_t GetLevel() const override { return data_.level; }
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::ObjectTypePlayer;
    }
    uint32_t GetInactiveTime() const
    {
        if (lastPing_ == 0)
            return 0;
        return static_cast<uint32_t>(Utils::AbTick() - lastPing_);
    }

    void Logout();
    void Ping();

    AB::Entities::Character data_;
    time_t loginTime_;
    time_t logoutTime_;
    std::string map_;
    int64_t lastPing_ = 0;
    std::shared_ptr<Net::ProtocolGame> client_;

};

}
