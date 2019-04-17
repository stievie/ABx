#pragma once

#include "../AICharacter.h"
#include "../Npc.h"
#include "../ResourceComp.h"
#include "../Mechanic.h"

namespace AI {

/**
 * @ingroup AI
 */
class IsSelfHealthCritical : public ai::ICondition
{
public:
    CONDITION_CLASS(IsSelfHealthCritical)
    CONDITION_FACTORY(IsSelfHealthCritical)

    bool evaluate(const ai::AIPtr& entity) override
    {
        const ai::Zone* zone = entity->getZone();
        if (zone == nullptr)
            return false;

        const AiCharacter& chr = entity->getCharacterCast<AiCharacter>();
        const auto& npc = chr.GetNpc();
        if (npc.IsDead())
            // Too late
            return false;
        return npc.resourceComp_.GetHealth() < CRITICAL_HP_THRESHOLD;
    }
};

}