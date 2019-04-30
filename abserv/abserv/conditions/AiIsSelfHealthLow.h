#pragma once

#include "../AiCharacter.h"
#include "../Npc.h"
#include "../ResourceComp.h"
#include "../Mechanic.h"

namespace AI {

/**
 * @ingroup AI
 */
class IsSelfHealthLow : public ai::ICondition
{
public:
    CONDITION_CLASS(IsSelfHealthLow)
    CONDITION_FACTORY(IsSelfHealthLow)

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
        return npc.resourceComp_.GetHealthRatio() < LOW_HP_THRESHOLD;
    }
};

}
