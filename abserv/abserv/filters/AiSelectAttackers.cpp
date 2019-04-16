#include "stdafx.h"
#include "AiSelectAttackers.h"
#include "../Npc.h"
#include "../AiCharacter.h"

namespace AI {

void SelectAttackers::filter(const ai::AIPtr& entity)
{
    ai::FilteredEntities& entities = getFilteredEntities(entity);

    std::map<uint32_t, float> sorting;
    Game::Npc& chr = getNpc(entity);
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor* o)
    {
        if (o->attackComp_.IsTarget(&chr))
        {
            entities.push_back(o->id_);
            sorting[o->id_] = o->GetDistance(&chr);
        }
    });
    std::sort(entities.begin(), entities.end(), [&sorting](int i, int j)
    {
        const float& p1 = sorting[i];
        const float& p2 = sorting[j];
        return p1 < p2;
    });
}

}