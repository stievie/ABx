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

/// Direction relative to current rotation
enum MoveDirection
{
    MoveDirectionNorth = 0,
    MoveDirectionNorthWest = 45,
    MoveDirectionWest = 90,
    MoveDirectionSouthWest = 135,
    MoveDirectionSouth = 180,
    MoveDirectionSouthEast = 225,
    MoveDirectionEast = 270,
    MoveDirectionNorthEast = 315,
};

/// DB Data
struct PlayerData
{
    uint32_t id;
    uint32_t accountId;
    /// Character name
    std::string name;
    /// PvP only character
    bool pvp;
    uint32_t level;
    uint64_t xp;
    uint32_t skillPoints;
    PlayerSex sex;
    std::string lastMap;
};

class Player final : public Creature
{
protected:
    friend class PlayerManager;
    std::shared_ptr<Player> GetThis()
    {
        return std::static_pointer_cast<Player>(shared_from_this());
    }
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

    void Logout();

    PlayerData data_;
    time_t loginTime_;
    time_t logoutTime_;
    std::string map_;
    int64_t lastPing_ = 0;
    std::shared_ptr<Net::ProtocolGame> client_;

};

}
