#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class FalseCondition : public Condition
{
public:
    CONDITON_CLASS(FalseCondition)
    explicit FalseCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&) override;
};

class TrueCondition : public Condition
{
public:
    CONDITON_CLASS(TrueCondition)
    explicit TrueCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&) override;
};

class AndCondition : public Condition
{
private:
    std::vector<std::shared_ptr<Condition>> conditions_;
public:
    CONDITON_CLASS(AndCondition)
    explicit AndCondition(const ArgumentsType& arguments);
    bool AddCondition(std::shared_ptr<Condition> condition) override;

    bool Evaluate(Agent&) override;
};

class OrCondition : public Condition
{
private:
    std::vector<std::shared_ptr<Condition>> conditions_;
public:
    CONDITON_CLASS(OrCondition)
    explicit OrCondition(const ArgumentsType& arguments);
    bool AddCondition(std::shared_ptr<Condition> condition) override;

    bool Evaluate(Agent&) override;
};

class NotCondition : public Condition
{
private:
    std::shared_ptr<Condition> condition_;
public:
    CONDITON_CLASS(NotCondition)
    explicit NotCondition(const ArgumentsType& arguments);
    bool AddCondition(std::shared_ptr<Condition> condition) override;

    bool Evaluate(Agent&) override;
};

}
}
