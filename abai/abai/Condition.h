#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include "Factory.h"

namespace AI {

class Agent;
class Condition;

struct ConditionFactoryContext
{
    std::vector<std::shared_ptr<Condition>> conditions;
};

using ConditionFactory = AbstractFactory<Condition, ConditionFactoryContext>;

#define CONDITON_FACTORY(ConditionName) \
	class Factory : public ConditionFactory                                                  \
    {                                                                                        \
	public:                                                                                  \
		std::shared_ptr<Condition> Create(const ConditionFactoryContext& ctx) const override \
        {                                                                                    \
			return std::make_shared<ConditionName>(ctx);                                     \
		}                                                                                    \
	};                                                                                       \
	static const Factory& GetFactory() {                                                     \
		static Factory sFactory;                                                             \
		return sFactory;                                                                     \
	}

class Condition
{
public:
    Condition();
    virtual ~Condition();

    virtual bool Evaluate(const Agent&) = 0;
};

}
