#include "stdafx.h"
#include "Agent.h"
#include "Zone.h"
#include "Root.h"
#include "Action.h"

namespace AI {

Agent::Agent(Id id) :
    id_(id)
{ }

Agent::~Agent() = default;

void Agent::Update(uint32_t timeElapsed)
{
    if (pause_)
        return;
    if (root_)
    {
        if (auto ca = context_.currentAction_.lock())
        {
            if (ca->MustComplete())
            {
                currentStatus_ = ca->Execute(*this, timeElapsed);
                return;
            }
        }
        currentStatus_ = root_->Execute(*this, timeElapsed);
    }
}

void Agent::SetBehavior(std::shared_ptr<Root> node)
{
    root_ = node;
}

std::shared_ptr<Root> Agent::GetBehavior() const
{
    return root_;
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
