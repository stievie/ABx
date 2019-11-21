#pragma once

#include "Action.h"
#include "Registry.h"
#include "Filter.h"

namespace AI {

class TestAction : public Action
{
    NODE_CLASS(TestAction)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit TestAction(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

class RunningAction : public Action
{
    NODE_CLASS(RunningAction)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit RunningAction(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

class Running2Action : public Action
{
    NODE_CLASS(Running2Action)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit Running2Action(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

class SelectSelf : public Filter
{
    FILTER_CLASS(SelectSelf)
public:
    explicit SelectSelf(const ArgumentsType& arguments) :
        Filter(arguments)
    { }
    void Execute(Agent& agent) override;
};

class SelectNothing : public Filter
{
    FILTER_CLASS(SelectNothing)
public:
    explicit SelectNothing(const ArgumentsType& arguments) :
        Filter(arguments)
    { }
    void Execute(Agent& agent) override;
};

class TestRegistry : public Registry
{
public:
    TestRegistry() :
        Registry()
    { }
    void Initialize() override;
};

}
