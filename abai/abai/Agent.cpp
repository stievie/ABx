#include "stdafx.h"
#include "Agent.h"
#include "Zone.h"
#include "Root.h"

namespace AI {

Agent::Agent(Id id) :
    id_(id)
{ }

Agent::~Agent() = default;

void Agent::Update(uint32_t timeElapsed)
{
    if (pause_)
        return;
    if (behavior_)
        currentStatus_ = behavior_->Execute(*this, timeElapsed);
}

void Agent::SetBehavior(std::shared_ptr<Root> node)
{
    behavior_ = node;
}

std::shared_ptr<Root> Agent::GetBehavior() const
{
    return behavior_;
}

Zone* Agent::GetZone() const
{
    return zone_;
}

void Agent::SetZone(Zone* zone)
{
    zone_ = zone;
}

}
