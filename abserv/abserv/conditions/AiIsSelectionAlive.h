#pragma once

#include "../AICharacter.h"
#include "../Npc.h"

namespace AI {

/**
 * @ingroup AI
 */
class IsSelectionAlive : public ai::ICondition
{
public:
    CONDITION_CLASS(IsSelectionAlive)
    CONDITION_FACTORY(IsSelectionAlive)

    bool evaluate(const ai::AIPtr& entity) override
    {
        const ai::Zone* zone = entity->getZone();
        if (zone == nullptr)
            return false;

        const ai::FilteredEntities& selection = entity->getFilteredEntities();
        if (selection.empty())
            return false;

        for (ai::CharacterId id : selection)
        {
            const ai::AIPtr& ai = zone->getAI(id);
            const AiCharacter& chr = ai->getCharacterCast<AiCharacter>();
            if (chr.GetNpc().IsDead())
                return false;
        }
        return true;
    }
};

}