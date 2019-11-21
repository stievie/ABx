#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class FalseCondition : public Condition
{
    CONDITON_CLASS(FalseCondition)
public:
    explicit FalseCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&, const Node&) override;
};

class TrueCondition : public Condition
{
    CONDITON_CLASS(TrueCondition)
public:
    explicit TrueCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&, const Node&) override;
};

class AndCondition : public Condition
{
    CONDITON_CLASS(AndCondition)
private:
    std::vector<std::shared_ptr<Condition>> conditions_;
public:
    explicit AndCondition(const ArgumentsType& arguments);
    bool AddCondition(std::shared_ptr<Condition> condition) override;

    bool Evaluate(Agent&, const Node&) override;
};

class OrCondition : public Condition
{
    CONDITON_CLASS(OrCondition)
private:
    std::vector<std::shared_ptr<Condition>> conditions_;
public:
    explicit OrCondition(const ArgumentsType& arguments);
    bool AddCondition(std::shared_ptr<Condition> condition) override;

    bool Evaluate(Agent&, const Node&) override;
};

class NotCondition : public Condition
{
    CONDITON_CLASS(NotCondition)
private:
    std::shared_ptr<Condition> condition_;
public:
    explicit NotCondition(const ArgumentsType& arguments);
    bool AddCondition(std::shared_ptr<Condition> condition) override;

    bool Evaluate(Agent&, const Node&) override;
};

}
}
