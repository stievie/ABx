#pragma once

#include "Component.h"
#include "InputQueue.h"

namespace Game {
namespace Components {

class InputComponent : public Component
{
public:
    Game::InputQueue queue_;
    void Update(uint32_t timeElapsed, GameObject& object) override;
};

}
}
