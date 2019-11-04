#pragma once

#include "Agent.h"
#include "Factory.h"

namespace AI {

class Filter;

using FilterFactory = AbstractFactory<Filter>;

#define FILTER_FACTORY(FilterName)                                                          \
    class Factory : public FilterFactory                                                    \
    {                                                                                       \
    public:                                                                                 \
        std::shared_ptr<Filter> Create(const ArgumentsType& arguments) const override       \
        {                                                                                   \
            return std::make_shared<FilterName>(arguments);                                 \
        }                                                                                   \
    };                                                                                      \
    static const Factory& GetFactory()                                                      \
    {                                                                                       \
        static Factory sFactory;                                                            \
        return sFactory;                                                                    \
    }

class Filter
{
protected:
    AgentIds& GetFiltered(Agent& agent)
    {
        return agent.filteredAgents_;
    }
public:
    explicit Filter(const ArgumentsType& arguments);
    virtual ~Filter();


    virtual void Execute(Agent&) = 0;
};

}
