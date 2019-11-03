#include "stdafx.h"
#include "LastFilter.h"

namespace AI {
namespace Filters {

void LastFilter::Execute(Agent& agent)
{
    auto& entities = GetFiltered(agent);
    if (entities.size() == 0)
        return;

    auto value = entities.back();
    entities.clear();
    entities.push_back(value);
}

}
}
