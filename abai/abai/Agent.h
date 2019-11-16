#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include "AiDefines.h"
#include "Node.h"
#include <set>
#include "Context.h"
#include <sa/StrongType.h>
#include "Action.h"

namespace AI {

class Root;
class Zone;

// To create distinct types
using limit_type = sa::StrongType<size_t, struct LimitTag>;
using timer_type = sa::StrongType<uint32_t, struct TimerTag>;
using counter_type = sa::StrongType<uint32_t, struct CounterTag>;

using AgentIds = std::vector<Id>;
// Once the BT is loaded it must not be modified, so the iterators are not invalidated.
// This is a bit unflexible when an application needs to store other types, but
// you could create a second context in the subclass.
class AgentContext : public Context<limit_type, timer_type, counter_type, Nodes::iterator>
{
public:
    std::weak_ptr<Action> currentAction_;
    inline bool IsActionRunning(Id id) const
    {
        if (auto ca = currentAction_.lock())
            return ca->GetId() == id;
        return false;
    }
};

class Agent
{
private:
    /// Game provided ID, not managed by the library, unlike the Node ID.
    Id id_;
    Zone* zone_{ nullptr };
protected:
    Node::Status currentStatus_{ Node::Status::Unknown };
    std::shared_ptr<Root> root_;
public:
    Agent(Id id);
    virtual ~Agent();

    void Update(uint32_t timeElapsed);

    void SetBehavior(std::shared_ptr<Root> node);
    std::shared_ptr<Root> GetBehavior() const;
    Id GetId() const { return id_; }
    Zone* GetZone() const;
    void SetZone(Zone* zone);
    Node::Status GetCurrentStatus() const { return currentStatus_; }

    bool pause_{ false };
    // Selected Agent IDs
    AgentIds filteredAgents_;
    AgentContext context_;
};

}
