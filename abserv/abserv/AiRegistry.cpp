#include "stdafx.h"
#include "AiRegistry.h"
#include "actions/AiAttackSelection.h"
#include "actions/AiDie.h"
#include "actions/AiIdle.h"
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

namespace AI {

AiRegistry::AiRegistry() :
    Registry()
{ }

void AiRegistry::Initialize()
{
    Registry::Initialize();

    UnregisterFilterFactory("Zone");
    RegisterNodeFactory("GoHome", GoHome::GetFactory());
    RegisterNodeFactory("Die", Die::GetFactory());
    RegisterNodeFactory("Idle", Idle::GetFactory());
    RegisterNodeFactory("AttackSelection", AttackSelection::GetFactory());
    RegisterNodeFactory("HealOther", HealOther::GetFactory());
    RegisterNodeFactory("HealSelf", HealSelf::GetFactory());
    RegisterNodeFactory("Say", Say::GetFactory());
    RegisterNodeFactory("MoveTo", MoveTo::GetFactory());
    RegisterNodeFactory("ResurrectSelection", ResurrectSelection::GetFactory());

    RegisterConditionFactory("IsSelectionAlive", Conditions::IsSelectionAlive::GetFactory());
    RegisterConditionFactory("IsCloseToSelection", Conditions::IsCloseToSelection::GetFactory());
    RegisterConditionFactory("IsSelfHealthLow", Conditions::IsSelfHealthLow::GetFactory());
    RegisterConditionFactory("SelfHealthCritical", Conditions::IsSelfHealthCritical::GetFactory());
    RegisterConditionFactory("IsAllyHealthLow", Conditions::IsAllyHealthLow::GetFactory());
    RegisterConditionFactory("AllyHealthCritical", Conditions::IsAllyHealthCritical::GetFactory());
    RegisterConditionFactory("IsAttacked", Conditions::IsAttacked::GetFactory());

    RegisterFilterFactory("SelectVisible", Filters::SelectVisible::GetFactory());
    RegisterFilterFactory("SelectAggro", Filters::SelectAggro::GetFactory());
    RegisterFilterFactory("SelectLowHealth", Filters::SelectLowHealth::GetFactory());
    RegisterFilterFactory("SelectAttackers", Filters::SelectAttackers::GetFactory());
    RegisterFilterFactory("SelectDeadAllies", Filters::SelectDeadAllies::GetFactory());
}

}
