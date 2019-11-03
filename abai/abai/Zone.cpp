#include "stdafx.h"
#include "Zone.h"
#include "Agent.h"
#include "Node.h"

namespace AI {

Zone::Zone(const std::string& name) :
    name_(name)
{ }

Zone::~Zone() = default;

bool Zone::AddAgent(std::shared_ptr<Agent> agent)
{
    if (!agent)
        return false;
    agents_.emplace(agent->GetId(), agent);
    agent->SetZone(this);
    return true;
}

bool Zone::RemoveAgent(std::shared_ptr<Agent> agent)
{
    if (!agent)
        return false;

    if (agent->GetZone() == this)
        agent->SetZone(nullptr);
    return agents_.erase(agent->GetId()) != 0;
}

void Zone::Update(uint32_t timeElapsed)
{
    for (auto& agent : agents_)
    {
        if (agent.second->pause_)
            continue;

        agent.second->Update(timeElapsed);
    }
}

}
