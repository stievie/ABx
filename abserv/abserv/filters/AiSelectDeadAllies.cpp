#include "stdafx.h"
#include "AiSelectDeadAllies.h"
#include "../Npc.h"
#include "Subsystems.h"
#include "Random.h"

//#define DEBUG_AI

namespace AI {
namespace Filters {

SelectDeadAllies::SelectDeadAllies(const ArgumentsType& arguments) :
    Filter(arguments)
{ }

void SelectDeadAllies::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    std::map<uint32_t, float> sorting;
    Game::Npc& chr = GetNpc(agent);
    chr.VisitAlliesInRange(Game::Ranges::Aggro, [&](const Game::Actor& o)
    {
        float negligence = GetSubsystem<Crypto::Random>()->GetFloat();
        if (negligence < 0.05f)
            // 5% chance to not seeing it
            return Iteration::Continue;
        if (o.IsDead())
        {
            entities.push_back(o.id_);
            sorting[o.id_] = o.GetDistance(&chr);
        }
        return Iteration::Continue;
    });
    std::sort(entities.begin(), entities.end(), [&sorting](uint32_t i, uint32_t j)
    {
        const float& p1 = sorting[i];
        const float& p2 = sorting[j];
        return p1 < p2;
    });
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
#ifdef DEBUG_AI
//    LOG_DEBUG << "Dead allies " << entities.size() << std::endl;
#endif
}

}
}
