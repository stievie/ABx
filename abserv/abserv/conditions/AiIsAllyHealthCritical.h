#pragma once

#include "../AICharacter.h"
#include "../Npc.h"
#include "../ResourceComp.h"
#include "../Mechanic.h"

namespace AI {

/**
 * @ingroup AI
 */
class IsAllyHealthCritical : public ai::ICondition
{
public:
    CONDITION_CLASS(IsAllyHealthCritical)
    CONDITION_FACTORY(IsAllyHealthCritical)

    bool evaluate(const ai::AIPtr& entity) override
    {
        const ai::Zone* zone = entity->getZone();
        if (zone == nullptr)
            return false;

        const AiCharacter& chr = entity->getCharacterCast<AiCharacter>();
        auto& npc = chr.GetNpc();
        bool result = false;
        npc.VisitAlliesInRange(Game::Ranges::Aggro, [&result](const Game::Actor* o)
        {
            if (result)
                return;
            if (!o->IsDead() && o->resourceComp_.GetHealth() < CRITICAL_HP_THRESHOLD)
                result = true;
        });

        return result;
    }
};

}