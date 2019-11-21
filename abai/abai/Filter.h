#pragma once

#include "Agent.h"
#include "Factory.h"

namespace AI {

class Filter;

using FilterFactory = AbstractFactory<Filter>;

#define FILTER_CLASS(FilterName)                                                      \
public:                                                                               \
    class Factory : public FilterFactory                                              \
    {                                                                                 \
    public:                                                                           \
        std::shared_ptr<Filter> Create(const ArgumentsType& arguments) const override \
        {                                                                             \
            std::shared_ptr<Filter> res = std::make_shared<FilterName>(arguments);    \
            return res;                                                               \
        }                                                                             \
    };                                                                                \
    static const Factory& GetFactory()                                                \
    {                                                                                 \
        static Factory sFactory;                                                      \
        return sFactory;                                                              \
    }                                                                                 \
    FilterName(const FilterName&) = delete;                                           \
    FilterName& operator=(const FilterName&) = delete;                                \
    FilterName(FilterName&&) = delete;                                                \
    FilterName& operator=(FilterName&&) = delete;                                     \
    const char* GetClassName() const override { return ABAI_STRINGIFY(FilterName); }

class Filter
{
protected:
    AgentIds& GetFiltered(Agent& agent)
    {
        return agent.filteredAgents_;
    }
    std::string name_;
public:
    explicit Filter(const ArgumentsType& arguments);
    virtual ~Filter();

    virtual const char* GetClassName() const = 0;
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& value) { name_ = value; }

    virtual void Execute(Agent&) = 0;
};

}
