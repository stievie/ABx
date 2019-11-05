#include "stdafx.h"
#include "FirstFilter.h"

namespace AI {
namespace Filters {

FirstFilter::FirstFilter(const ArgumentsType& arguments) :
    Filter(arguments)
{ }

void FirstFilter::Execute(Agent& agent)
{
    auto& entities = GetFiltered(agent);
    if (entities.size() == 0)
        return;

    auto value = entities.front();
    entities.clear();
    entities.push_back(value);
}

}
}
