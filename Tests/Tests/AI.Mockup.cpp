#include "stdafx.h"
#include "AI.Mockup.h"
#include <iostream>

namespace AI {

Node::Status TestAction::DoAction(Agent&, uint32_t)
{
    std::cout << "TextAction::DoAction()" << std::endl;
    return Status::Finished;
}

void TestRegistry::Initialize()
{
    Registry::Initialize();
    RegisterNodeFactory("TestAction", TestAction::GetFactory());
    RegisterNodeFactory("RunningAction", RunningAction::GetFactory());
    RegisterNodeFactory("Running2Action", Running2Action::GetFactory());
    RegisterFilterFactory("SelectSelf", SelectSelf::GetFactory());
    RegisterFilterFactory("SelectNothing", SelectNothing::GetFactory());
}

void SelectSelf::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    entities.push_back(agent.GetId());
}

void SelectNothing::Execute(Agent& agent)
{
    agent.filteredAgents_.clear();
}

Node::Status RunningAction::DoAction(Agent&, uint32_t)
{
    return Status::Running;
}

Node::Status Running2Action::DoAction(Agent& agent, uint32_t)
{
    auto it = agent.counters_.find(id_);
    uint32_t runs = (it != agent.counters_.end() ? (*it).second : 0);
    ++runs;
    if (runs == 1)
    {
        agent.counters_[id_] = runs;
        return Status::Running;
    }
    agent.counters_[id_] = runs;
    return Status::Finished;
}

}
