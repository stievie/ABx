#include "stdafx.h"
#include "AiSelectVisible.h"
#include "../Npc.h"
#include "../AiCharacter.h"

namespace AI {

void SelectVisible::filter(const ai::AIPtr& entity)
{
    ai::FilteredEntities& entities = getFilteredEntities(entity);
    Game::Npc& chr = getNpc(entity);
    // So, raycast to all objects in range?
//	chr.visitVisible([&] (const EntityPtr& e) {
//		entities.push_back(e->id());
//	});
}

}