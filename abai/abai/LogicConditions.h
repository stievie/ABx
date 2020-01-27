/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
