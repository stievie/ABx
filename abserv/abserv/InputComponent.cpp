#include "stdafx.h"
#include "InputComponent.h"

namespace Game {
namespace Components {

void InputComponent::Update(uint32_t timeElapsed, GameObject& object)
{
    InputItem input;
    // Multiple inputs of the same type overwrite previous
    while (queue_.Get(input))
    {

    }
}

}
}
