#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include "Factory.h"
#ifdef DEBUG_AI
#include "Logger.h"
#endif // DEBUG_AI

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
            auto res = std::make_shared<ConditionName>(arguments);                           \
            res->SetType(ABAI_STRINGIFY(ConditionName));                                     \
            return res;                                                                      \
        }                                                                                    \
    };                                                                                       \
    static const Factory& GetFactory()                                                       \
    {                                                                                        \
        static Factory sFactory;                                                             \
        return sFactory;                                                                     \
    }

class Condition
{
protected:
    std::string type_;
    std::string name_;
public:
    explicit Condition(const ArgumentsType& arguments);
    virtual ~Condition();

    const std::string& GetType() const { return type_; }
    void SetType(const std::string& value) { type_ = value; }
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& value) { name_ = value; }

    virtual bool AddCondition(std::shared_ptr<Condition>);
    virtual bool SetFilter(std::shared_ptr<Filter>);
    virtual bool Evaluate(Agent&) = 0;
};

}
