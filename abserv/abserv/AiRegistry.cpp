#include "stdafx.h"
#include "AiRegistry.h"
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4189 4100)
#endif
#include "actions/AiAttackSelection.h"
#include "actions/AiDie.h"
#include "actions/AiSay.h"
#include "actions/AiGoHome.h"
#include "actions/AiMoveTo.h"
#include "actions/AiHealSelf.h"
#include "actions/AiHealOther.h"
#include "actions/AiResurrectSelection.h"
#include "conditions/AiIsSelectionAlive.h"
#include "conditions/AiIsCloseToSelection.h"
#include "conditions/AiIsSelfHealthLow.h"
#include "conditions/AiIsSelfHealthCritical.h"
#include "conditions/AiIsAllyHealthLow.h"
#include "conditions/AiIsAllyHealthCritical.h"
#include "conditions/AiIsAttacked.h"
#include "filters/AiSelectVisible.h"
#include "filters/AiSelectAggro.h"
#include "filters/AiSelectLowHealth.h"
#include "filters/AiSelectAttackers.h"
#include "filters/AiSelectDeadAllies.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace AI {

void AiRegistry::Initialize()
{
    // https://github.com/mgerhardy/engine/blob/master/src/modules/backend/entity/ai/AIRegistry.cpp
    registerNodeFactory("GoHome", GoHome::getFactory());
    registerNodeFactory("AttackSelection", AttackSelection::getFactory());
    registerNodeFactory("Die", Die::getFactory());
    registerNodeFactory("Say", Say::getFactory());
    registerNodeFactory("MoveTo", MoveTo::getFactory());
    registerNodeFactory("HealSelf", HealSelf::getFactory());
    registerNodeFactory("HealOther", HealOther::getFactory());
    registerNodeFactory("ResurrectSelection", ResurrectSelection::getFactory());

    registerConditionFactory("IsSelectionAlive", IsSelectionAlive::getFactory());
    registerConditionFactory("IsCloseToSelection", IsCloseToSelection::getFactory());
    registerConditionFactory("IsSelfHealthLow", IsSelfHealthLow::getFactory());
    registerConditionFactory("SelfHealthCritical", IsSelfHealthCritical::getFactory());
    registerConditionFactory("IsAllyHealthLow", IsAllyHealthLow::getFactory());
    registerConditionFactory("AllyHealthCritical", IsAllyHealthCritical::getFactory());
    registerConditionFactory("IsAttacked", IsAttacked::getFactory());

    registerFilterFactory("SelectVisible", SelectVisible::getFactory());
    registerFilterFactory("SelectAggro", SelectAggro::getFactory());
    registerFilterFactory("SelectLowHealth", SelectLowHealth::getFactory());
    registerFilterFactory("SelectAttackers", SelectAttackers::getFactory());
    registerFilterFactory("SelectDeadAllies", SelectDeadAllies::getFactory());
}

}
