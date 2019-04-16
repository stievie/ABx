#include "stdafx.h"
#include "AiSelectAggro.h"
#include "../Npc.h"
#include "../AiCharacter.h"

namespace AI {

void SelectAggro::filter(const ai::AIPtr& entity)
{
    ai::FilteredEntities& entities = getFilteredEntities(entity);
    Game::Npc& chr = getNpc(entity);
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor* o)
    {
        entities.push_back(o->id_);
    });
}

}