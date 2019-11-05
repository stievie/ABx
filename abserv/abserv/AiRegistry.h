#pragma once

#include "Registry.h"

namespace AI {

class AiRegistry : public Registry
{
public:
    AiRegistry();
    void Initialize() override;
};

}
