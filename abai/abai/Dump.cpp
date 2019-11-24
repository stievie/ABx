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
