#include "stdafx.h"
#include "AiSelectRandom.h"
#include "Random.h"
#include "Subsystems.h"
#include "Utils.h"

namespace AI {
namespace Filters {

SelectRandom::SelectRandom(const ArgumentsType& arguments) :
    Filter(arguments)
{
    if (arguments.size() != 0)
        count_ = atoi(arguments[0].c_str());
    else
        count_ = 1;
}

void SelectRandom::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    if (count_ == 0)
        return;
    if (count_ >= entities.size())
        return;

    auto* rnd = GetSubsystem<Crypto::Random>();
    AI::AgentIds copy(entities);
    entities.clear();
    while (entities.size() < count_)
    {
        const float rn = rnd->GetFloat();
        auto it = Utils::SelectRandomly(copy.begin(), copy.end(), rn);
        entities.push_back(*it);
        copy.erase(it);
    }
}

}
}
