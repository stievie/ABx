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

#include "stdafx.h"
#include "Dump.h"

namespace AI {

static void DumpCondition(std::ostream& stream, const Condition& condition)
{
    stream << "if (" << condition.GetClassName() << ") ";
}

void DumpTree(std::ostream& stream, const Node& node)
{
    static int indent = 0;
    for (int i = 0; i < indent; ++i)
        stream << "  ";
    if (const auto* cond = node.GetCondition())
    {
        DumpCondition(stream, *cond);
    }

    stream << node.GetClassName() << ":" << node.GetId();
    stream << std::endl;
    ++indent;
    node.VisitChildren([&](const Node& node)
    {
        DumpTree(stream, node);
        return Iteration::Continue;
    });
    --indent;
}

void DumpCache(std::ostream& stream, const BevaviorCache& cache)
{
    cache.VisitBehaviors([&](const std::string& name, const Root& root)
    {
        stream << name << ":" << std::endl;
        DumpTree(stream, root);
        stream << std::endl;
        return Iteration::Continue;
    });
}

}
