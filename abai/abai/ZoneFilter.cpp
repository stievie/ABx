#include "stdafx.h"
#include "ZoneFilter.h"
#include "Zone.h"
#include <sa/Iteration.h>

namespace AI {
namespace Filters {

ZoneFilter::ZoneFilter(const ArgumentsType& arguments) :
    Filter(arguments)
{ }

void ZoneFilter::Execute(Agent& agent)
{
    Zone* zone = agent.GetZone();
    if (!zone)
        return;

    auto& entities = GetFiltered(agent);
    zone->VisitAgents([&](const Agent& agent) {
        entities.push_back(agent.GetId());
        return Iteration::Continue;
    });
}

}
}
