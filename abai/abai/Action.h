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

#include "Node.h"

namespace AI {

// Do something useful, e.g. attacking mean foes. AKA Leaf.
// There is no factory for this because it's meant to be subclassed.
class Action : public Node, public std::enable_shared_from_this<Action>
{
protected:
    // If true, this Action is executed until it returns not running without
    // reevaluating the tree.
    bool mustComplete_{ false };
    bool IsCurrentAction(const Agent& agent) const;
    void SetCurrentAction(Agent& agent);
    void UnsetCurrentAction(Agent& agent);
    virtual Status DoAction(Agent& agent, uint32_t timeElapsed) = 0;
public:
    explicit Action(const ArgumentsType& arguments);
    ~Action() override;

    bool MustComplete() const { return mustComplete_; }
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
