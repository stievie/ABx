#pragma once

#include "ProtocolGame.h"
#include <memory>
#include "GameObject.h"
#include "Game.h"

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
    uint32_t xp;
    uint32_t skillPoints;
    PlayerSex sex;
};

class Player final : public GameObject
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
    ~Player();
    // non-copyable
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    void SetGame(std::shared_ptr<Game> game)
    {
        game_ = game;
    }

    PlayerData data_;
    /// Auto ID, not DB ID
    uint32_t id_;
    std::shared_ptr<Net::ProtocolGame> client_;
};

}
