#include "stdafx.h"
#include "GameManager.h"

namespace Game {
GameManager GameManager::Instance;

GameManager::GameManager()
{
}


GameManager::~GameManager()
{
}

void GameManager::Start(Net::ServiceManager* serviceManager)
{
    serviceManager_ = serviceManager;
}

}
