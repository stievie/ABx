#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include "AiDefines.h"
#include "Node.h"
#include <set>

namespace AI {

class Root;
class Zone;

typedef std::vector<Id> AgentIds;

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
    bool IsActionRunning(Id id) const { return runningActions_.find(id) != runningActions_.end(); }

    bool pause_{ false };
    AgentIds filteredAgents_;
    std::set<Id> runningActions_;
    std::unordered_map<Id, size_t> limits_;
};

}
