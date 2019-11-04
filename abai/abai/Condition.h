#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include "Factory.h"

namespace AI {

class Agent;
class Condition;
class Filter;

using ConditionFactory = AbstractFactory<Condition>;

#define CONDITON_FACTORY(ConditionName)                                                      \
    class Factory : public ConditionFactory                                                  \
    {                                                                                        \
    public:                                                                                  \
        std::shared_ptr<Condition> Create(const ArgumentsType& arguments) const override     \
        {                                                                                    \
            return std::make_shared<ConditionName>(arguments);                               \
        }                                                                                    \
    };                                                                                       \
    static const Factory& GetFactory()                                                       \
    {                                                                                        \
        static Factory sFactory;                                                             \
        return sFactory;                                                                     \
    }

class Condition
{
public:
    explicit Condition(const ArgumentsType& arguments);
    virtual ~Condition();

    virtual bool AddCondition(std::shared_ptr<Condition>);
    virtual bool SetFilter(std::shared_ptr<Filter>);
    virtual bool Evaluate(Agent&) = 0;
};

}
