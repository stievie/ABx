#include "stdafx.h"
#include "AiRegistry.h"
#pragma warning(push)
#pragma warning(disable: 4189 4100)
#include "AiAttackOnSelection.h"
#pragma warning(pop)

namespace AI {

void AiRegistry::Initialize()
{
    // https://github.com/mgerhardy/engine/blob/master/src/modules/backend/entity/ai/AIRegistry.cpp
    registerNodeFactory("AttackOnSelection", AttackOnSelection::getFactory());
}

}
