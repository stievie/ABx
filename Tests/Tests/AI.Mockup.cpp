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

}
