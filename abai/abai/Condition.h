#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include "Factory.h"

namespace AI {

class Agent;
class Condition;
class Filter;
class Node;

using ConditionFactory = AbstractFactory<Condition>;

#define CONDITON_CLASS(ConditionName)                                                    \
    class Factory : public ConditionFactory                                              \
    {                                                                                    \
    public:                                                                              \
        std::shared_ptr<Condition> Create(const ArgumentsType& arguments) const override \
        {                                                                                \
            std::shared_ptr<Condition> res = std::make_shared<ConditionName>(arguments); \
            return res;                                                                  \
        }                                                                                \
    };                                                                                   \
    static const Factory& GetFactory()                                                   \
    {                                                                                    \
        static Factory sFactory;                                                         \
        return sFactory;                                                                 \
    }                                                                                    \
    ConditionName(const ConditionName&) = delete;                                        \
    ConditionName& operator=(const ConditionName&) = delete;                             \
    ConditionName(ConditionName&&) = delete;                                             \
    ConditionName& operator=(ConditionName&&) = delete;                                  \
    const char* GetClassName() const override { return ABAI_STRINGIFY(ConditionName); }

class Condition
{
protected:
    std::string name_;
public:
    explicit Condition(const ArgumentsType& arguments);
    virtual ~Condition();

    virtual const char* GetClassName() const = 0;
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& value) { name_ = value; }

    virtual bool AddCondition(std::shared_ptr<Condition>);
    virtual bool SetFilter(std::shared_ptr<Filter>);
    /// Evaluate the condition
    /// @param agent
    /// @param node The calling node
    virtual bool Evaluate(Agent& agent, const Node& node) = 0;
};

}
