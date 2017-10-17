#pragma once

#include "Service.h"

class GameManager
{
private:
    Net::ServiceManager* serviceManager_;
public:
    GameManager();
    ~GameManager();

    void Start(Net::ServiceManager* serviceManager);
public:
    static GameManager Instance;
};

