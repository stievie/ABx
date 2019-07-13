#pragma once

#include "../AiCharacter.h"
#include "../Npc.h"
#include "../ResourceComp.h"
#include "../Mechanic.h"

namespace AI {

/**
 * @ingroup AI
 */
class IsAttacked : public ai::ICondition
{
public:
    CONDITION_CLASS(IsAttacked)
    CONDITION_FACTORY(IsAttacked)

    bool evaluate(const ai::AIPtr& entity) override
    {
        const ai::Zone* zone = entity->getZone();
        if (zone == nullptr)
            return false;

        const AiCharacter& chr = entity->getCharacterCast<AiCharacter>();
        if (Utils::TimeElapsed(chr.lastFoeAttack_) < 1000)
        {
            if (auto attacker = chr.lastAttacker_.lock())
            {
                if (attacker->IsDead())
                    return false;
                return true;
            }
        }
        return false;
    }
};

}
