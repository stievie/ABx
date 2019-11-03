#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class FalseCondition : public Condition
{
public:
    CONDITON_FACTORY(FalseCondition)
    FalseCondition(const ConditionFactoryContext& ctx);
    bool Evaluate(const Agent&) override;
};

class TrueCondition : public Condition
{
public:
    CONDITON_FACTORY(TrueCondition)
    TrueCondition(const ConditionFactoryContext& ctx);
    bool Evaluate(const Agent&) override;
};

class AndCondition : public Condition
{
private:
    std::vector<std::shared_ptr<Condition>> conditions_;
public:
    CONDITON_FACTORY(AndCondition)
    AndCondition(const ConditionFactoryContext& ctx);

    bool Evaluate(const Agent&) override;
};

class OrCondition : public Condition
{
private:
    std::vector<std::shared_ptr<Condition>> conditions_;
public:
    CONDITON_FACTORY(OrCondition)
    OrCondition(const ConditionFactoryContext& ctx);

    bool Evaluate(const Agent&) override;
};

class NotCondition : public Condition
{
private:
    std::shared_ptr<Condition> condition_;
public:
    CONDITON_FACTORY(NotCondition)
    NotCondition(const ConditionFactoryContext& ctx);

    bool Evaluate(const Agent&) override;
};

}
}
