#pragma once

#include "Node.h"
#include "Filter.h"
#include "Condition.h"
#include <memory>

namespace AI {

class Registry
{
protected:
    using NodeFactoryRegistry = FactoryRegistry<std::string, Node, NodeFactoryContext>;
    using FilterFactoryRegistry = FactoryRegistry<std::string, Filter, FilterFactoryContext>;
    using ConditionFactoryRegistry = FactoryRegistry<std::string, Condition, ConditionFactoryContext>;

    NodeFactoryRegistry nodeFactory_;
    FilterFactoryRegistry filterFactory_;
    ConditionFactoryRegistry conditionFactory_;
public:
    Registry();
    ~Registry() = default;

    bool RegisterNodeFactory(const std::string& name, const NodeFactory& factory);
    bool UnregisterNodeFactory(const std::string& name);
    bool RegisterFilterFactory(const std::string& name, const FilterFactory& factory);
    bool UnregisterFilterFactory(const std::string& name);
    bool RegisterConditionFactory(const std::string& name, const ConditionFactory& factory);
    bool UnregisterConditionFactory(const std::string& name);

    std::shared_ptr<Node> CreateNode(const std::string& nodeType, const NodeFactoryContext& ctx);
    std::shared_ptr<Filter> CreateFilter(const std::string& filterType, const FilterFactoryContext& ctx);
    std::shared_ptr<Condition> CreateCondition(const std::string& conditionType, const ConditionFactoryContext& ctx);
};

}
