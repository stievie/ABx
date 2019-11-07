#pragma once

#include <stdint.h>
#include <unordered_map>
#include <memory>
#include <sa/Iteration.h>
#include "AiDefines.h"

namespace AI {

class Agent;

// In ABx we don't use a Zone, the NPC should call Agent::Update()
class Zone
{
private:
    std::unordered_map<Id, std::shared_ptr<Agent>> agents_;
public:
    Zone(const std::string& name);
    ~Zone();

    bool AddAgent(std::shared_ptr<Agent> agent);
    bool RemoveAgent(std::shared_ptr<Agent> agent);

    void Update(uint32_t timeElapsed);

    std::string name_;

    template<typename Callback>
    inline void VisitAgents(Callback callback);
};

template<typename Callback>
inline void Zone::VisitAgents(Callback callback)
{
    for (auto agent : agents_)
    {
        if (callback(*agent.second) != Iteration::Continue)
            break;
    }
}

}
