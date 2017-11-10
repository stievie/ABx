#pragma once

#include "ProtocolGame.h"
#include <memory>
#include "GameObject.h"
#include "Game.h"
#include "Effect.h"
#include <list>
#include "Creature.h"

namespace Game {

class PlayerManager;

enum PlayerSex
{
    PlayerSexFemale,
    PlayerSexMale
};

/// DB Data
struct PlayerData
{
    uint32_t id;
    uint32_t accountId;
    /// Character name
    std::string name;
    uint32_t level;
    uint64_t xp;
    uint32_t skillPoints;
    PlayerSex sex;
    std::string lastMap;
    time_t lastLogin;
    time_t lastLogout;
};

class Player final : public Creature
{
private:
    std::weak_ptr<Game> game_;
protected:
    friend class PlayerManager;
    std::shared_ptr<Player> GetThis()
    {
        return std::static_pointer_cast<Player>(shared_from_this());
    }
    explicit Player(std::shared_ptr<Net::ProtocolGame> client);
public:
    static void RegisterLua(kaguya::State& state);

    ~Player() override;
    // non-copyable
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    std::shared_ptr<Game> GetGame() const
    {
        return game_.lock();
    }
    void SetGame(std::shared_ptr<Game> game)
    {
        game_ = game;
    }

    PlayerData data_;
    time_t loginTime_;
    time_t logoutTime_;
    std::shared_ptr<Net::ProtocolGame> client_;
};

}
