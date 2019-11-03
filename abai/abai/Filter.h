#pragma once

#include "Agent.h"
#include "Factory.h"

namespace AI {

class Filter;
struct FilterFactoryContext
{
};

using FilterFactory = AbstractFactory<Filter, FilterFactoryContext>;

#define FILTER_FACTORY(FilterName) \
	class Factory : public FilterFactory                                                    \
    {                                                                                       \
	public:                                                                                 \
		std::shared_ptr<Filter> Create(const FilterFactoryContext&) const override          \
        {                                                                                   \
			return std::make_shared<FilterName>();                                          \
		}                                                                                   \
	};                                                                                      \
	static const Factory& GetFactory() {                                                    \
		static Factory sFactory;                                                            \
		return sFactory;                                                                    \
	}

class Filter
{
protected:
    AgentIds& GetFiltered(Agent& agent) {
        return agent.filteredAgents_;
    }
public:
    Filter();
    virtual ~Filter();

    virtual void Execute(Agent&) = 0;
};

}
