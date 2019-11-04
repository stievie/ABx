#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include "AiTypes.h"
#include "Node.h"

namespace AI {

class Root;
class Zone;

typedef std::vector<Id> AgentIds;

class Agent
{
private:
    /// Game provided ID, not managed by the library.
    Id id_;
    Zone* zone_{ nullptr };
protected:
    Node::Status currentStatus_{ Node::Status::Unknown };
    std::shared_ptr<Root> behavior_;
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
    mutable AgentIds filteredAgents_;
    Id currentAction_{ INVALID_ID };
    std::unordered_map<Id, size_t> limits_;
};

}
