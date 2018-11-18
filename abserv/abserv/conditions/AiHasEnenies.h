#pragma once

#include "../AICharacter.h"
#include "../Npc.h"

namespace AI {

/**
 * @ingroup AI
 */
class HasEnemies : public ai::ICondition
{
public:
    CONDITION_CLASS(HasEnemies)
    CONDITION_FACTORY(HasEnemies)

    bool evaluate(const ai::AIPtr& entity) override
    {
        const ai::Zone* zone = entity->getZone();
        if (zone == nullptr)
            return false;

        const ai::FilteredEntities& selection = entity->getFilteredEntities();
        if (selection.empty())
            return false;

        const AiCharacter& chrMe = entity->getCharacterCast<AiCharacter>();
        Game::Npc& me = chrMe.GetNpc();
        for (ai::CharacterId id : selection)
        {
            const ai::AIPtr& ai = zone->getAI(id);
            const AiCharacter& chr = ai->getCharacterCast<AiCharacter>();
            if (me.IsEnemy(&chr.GetNpc()))
            {
#ifdef DEBUG_AI
                LOG_DEBUG << me.GetName() << " has enemy " << chr.GetNpc().GetName() << std::endl;
#endif
                return true;
            }
        }
        return false;
    }
};

}