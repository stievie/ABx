#include "stdafx.h"
#include "Game.h"
#include "Utils.h"

#include "DebugNew.h"

namespace Game {

Game::Game()
{
    startTime_ = Utils::AbTick();
}

Game::~Game()
{
}

void Game::Update(uint32_t timeElapsed)
{
    for (const auto& o : objets_)
    {
        o->Update(timeElapsed);
    }
}

}
