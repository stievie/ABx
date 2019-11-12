#pragma once

#include "Action.h"
#include "Registry.h"
#include "Filter.h"

namespace AI {

class TestAction : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(TestAction)
    explicit TestAction(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

class RunningAction : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(RunningAction)
    explicit RunningAction(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

class Running2Action : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(Running2Action)
    explicit Running2Action(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

class SelectSelf : public Filter
{
public:
    FILTER_CLASS(SelectSelf)
    explicit SelectSelf(const ArgumentsType& arguments) :
        Filter(arguments)
    { }
    void Execute(Agent& agent) override;
};

class SelectNothing : public Filter
{
public:
    FILTER_CLASS(SelectNothing)
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
