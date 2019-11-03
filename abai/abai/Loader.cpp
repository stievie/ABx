#include "stdafx.h"
#include "Loader.h"
#include "Root.h"
#include "Condition.h"
#include "Filter.h"

namespace AI {

Loader::Loader(Registry& reg) :
    registry_(reg)
{
}

void Loader::RegisterLua(kaguya::State& state)
{
    state["Node"].setClass(kaguya::UserdataMetatable<Node>()
        .addFunction("SetCondition", &Node::SetCondition)
    );

    state["Loader"].setClass(kaguya::UserdataMetatable<Loader>()
        .addFunction("AddNode", &Loader::AddNode)
        .addFunction("CreateCondition", &Loader::CreateCondition)
        .addFunction("CreateFilter", &Loader::CreateFilter)
    );
}

std::shared_ptr<Node> Loader::AddNode(Node* parent, const std::string& type)
{
    if (!parent)
        return std::shared_ptr<Node>();

    NodeFactoryContext ctx;
    auto node = registry_.CreateNode(type, ctx);
    if (parent->AddChild(node))
        return node;

    return std::shared_ptr<Node>();
}

std::shared_ptr<Condition> Loader::CreateCondition(const std::string& type)
{
    ConditionFactoryContext ctx;
    auto cond = registry_.CreateCondition(type, ctx);
    return cond;
}

std::shared_ptr<Filter> Loader::CreateFilter(const std::string& type)
{
    FilterFactoryContext ctx;
    return registry_.CreateFilter(type, ctx);
}

std::shared_ptr<Root> Loader::Load(const std::string& fileName)
{
    kaguya::State luaState;
    Loader::RegisterLua(luaState);
    luaState["self"] = this;
    NodeFactoryContext ctx;
    std::shared_ptr<Root> result = std::make_shared<Root>(ctx);
    luaState["root"] = result.get();

    if (!luaState.dofile(fileName.c_str()))
    {
        return std::shared_ptr<Root>();
    }

    return result;
}

}
