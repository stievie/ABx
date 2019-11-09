#pragma once

#include "Agent.h"
#include "Factory.h"
#ifdef DEBUG_AI
#include "Logger.h"
#endif // DEBUG_AI

namespace AI {

class Filter;

using FilterFactory = AbstractFactory<Filter>;

#define FILTER_FACTORY(FilterName)                                                          \
    class Factory : public FilterFactory                                                    \
    {                                                                                       \
    public:                                                                                 \
        std::shared_ptr<Filter> Create(const ArgumentsType& arguments) const override       \
        {                                                                                   \
            std::shared_ptr<Filter> res = std::make_shared<FilterName>(arguments);          \
            res->SetType(ABAI_STRINGIFY(FilterName));                                       \
            return res;                                                                     \
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
    std::string type_;
    std::string name_;
public:
    explicit Filter(const ArgumentsType& arguments);
    virtual ~Filter();

    const std::string& GetType() const { return type_; }
    void SetType(const std::string& value) { type_ = value; }
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& value) { name_ = value; }

    virtual void Execute(Agent&) = 0;
};

}
