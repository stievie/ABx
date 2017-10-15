#pragma once

#include "Service.h"

class GameManager
{
private:
    ServiceManager* serviceManager_;
public:
    GameManager();
    ~GameManager();

    void Start(ServiceManager* serviceManager);
public:
    static GameManager Instance;
};

