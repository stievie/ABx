#include "stdafx.h"
#include "AiRegistry.h"
#pragma warning(push)
#pragma warning(disable: 4189 4100)
#include "actions/AiAttackOnSelection.h"
#include "actions/AiDie.h"
#include "actions/AiGoHome.h"
#include "conditions/AiIsSelectionAlive.h"
#include "conditions/AiIsCloseToSelection.h"
#include "filters/AiSelectVisible.h"
#pragma warning(pop)

namespace AI {

void AiRegistry::Initialize()
{
    // https://github.com/mgerhardy/engine/blob/master/src/modules/backend/entity/ai/AIRegistry.cpp
    registerNodeFactory("GoHome", GoHome::getFactory());
    registerNodeFactory("AttackOnSelection", AttackOnSelection::getFactory());
    registerNodeFactory("Die", Die::getFactory());

    registerConditionFactory("IsSelectionAlive", IsSelectionAlive::getFactory());
    registerConditionFactory("IsCloseToSelection", IsCloseToSelection::getFactory());

    registerFilterFactory("SelectVisible", SelectVisible::getFactory());
}

}
