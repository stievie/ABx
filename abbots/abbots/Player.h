#pragma once

#include "GameObject.h"
#include <kaguya/kaguya.hpp>

class Player final : public GameObject
{
private:
    kaguya::State luaState_;
public:
    Player(Type type, uint32_t id);
    void Update(uint32_t timeElapsed) override;
};
