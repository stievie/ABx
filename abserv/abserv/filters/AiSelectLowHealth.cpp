#include "stdafx.h"
#include "AiSelectLowHealth.h"
#include "../Npc.h"
#include "../AiCharacter.h"

namespace AI {

void SelectLowHealth::filter(const ai::AIPtr& entity)
{
    ai::FilteredEntities& entities = getFilteredEntities(entity);
    Game::Npc& chr = getNpc(entity);
    std::map<uint32_t, std::pair<float, float>> sorting;

    chr.VisitAlliesInRange(Game::Ranges::Aggro, [&](const Game::Actor* o)
    {
        if (o->resourceComp_->GetHealthRatio() < LOW_HP_THRESHOLD)
        {
            entities.push_back(o->id_);
            sorting[o->id_] = std::make_pair<float, float>(o->resourceComp_->GetHealthRatio(), o->GetDistance(&chr));
        }
    });
    std::sort(entities.begin(), entities.end(), [&sorting](int i, int j)
    {
        const std::pair<float, float>& p1 = sorting[i];
        const std::pair<float, float>& p2 = sorting[j];
        if (fabs(p1.first - p2.first) < 0.05f)
            // If same HP (max 5% difference) use shorter distance
            return p1.second < p2.second;
        return p1.first < p2.first;
    });
}

}