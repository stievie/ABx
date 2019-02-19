#include "stdafx.h"
#include "AiSelectAggro.h"
#include "../Npc.h"
#include "../AiCharacter.h"

namespace AI {

void SelectAggro::filter(const ai::AIPtr& entity)
{
    ai::FilteredEntities& entities = getFilteredEntities(entity);
    Game::Npc& chr = getNpc(entity);
	chr.VisitInRange(Game::Ranges::Aggro, [&](const std::shared_ptr<Game::GameObject>& o)
    {
        entities.push_back(o->id_);
    });
}

}