#include "stdafx.h"
#include "AiSelectVisible.h"
#include "../Npc.h"
#include "../AiCharacter.h"

namespace AI {

void SelectVisible::filter(const ai::AIPtr& entity)
{
    ai::FilteredEntities& entities = getFilteredEntities(entity);
    Game::Npc& chr = getNpc(entity);
    chr.VisitInRange(Game::Ranges::Compass, [&](const Game::GameObject& o)
    {
        entities.push_back(o.id_);
        return Iteration::Continue;
    });
}

}