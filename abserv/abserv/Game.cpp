#include "stdafx.h"
#include "Game.h"
#include "PlayerManager.h"
#include "Player.h"

#include "DebugNew.h"

namespace Game {

Game::Game()
{
    startTime_ = Utils::AbTick();
}

void Game::Update(uint32_t timeElapsed)
{
    for (const auto& o : objets_)
    {
        o->Update(timeElapsed);
    }
}

Player* Game::GetPlayerById(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it == players_.end())
        return nullptr;
    return (*it).second;
}

void Game::PlayerJoin(uint32_t playerId)
{
    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (player)
    {
        players_[player->id_] = player.get();
        player->SetGame(shared_from_this());
        objets_.push_back(player);
    }
}

void Game::PlayerMove(uint32_t playerId, MoveDirection direction)
{
    Player* player = GetPlayerById(playerId);
    if (!player)
        return;

}

}
