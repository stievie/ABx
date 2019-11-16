#include "stdafx.h"
#include "Loader.h"
#include "Root.h"
#include "Condition.h"
#include "Filter.h"
#include <sa/StringUtils.h>
#include "BevaviorCache.h"

namespace AI {

Loader::Loader(Registry& reg) :
    registry_(reg)
{ }

Loader::~Loader() = default;

bool Loader::ExecuteScript(kaguya::State& state, const std::string& file)
{
    const std::string filename = GetScriptFile(file);
    if (filename.empty())
        return false;
    return state.dofile(filename.c_str());
}

void Loader::RegisterLua(kaguya::State& state)
{
    state["include"] = kaguya::function([this, &state](const std::string& file)
    {
        std::string ident(file);
        sa::MakeIdent(ident);
        ident = "__included_" + ident + "__";
        if (state[ident].type() == LUA_TBOOLEAN)
            return;

        if (ExecuteScript(state, file))
            state[ident] = true;
    });

    state["Node"].setClass(kaguya::UserdataMetatable<Node>()
        .addFunction("SetCondition", &Node::SetCondition)
        .addFunction("AddNode", &Node::AddNode)
        .addFunction("SetName", &Node::SetName)
    );
    state["Root"].setClass(kaguya::UserdataMetatable<Root, Node>()
    );

    state["Condition"].setClass(kaguya::UserdataMetatable<Condition>()
        .addFunction("AddCondition", &Condition::AddCondition)
        .addFunction("SetFilter", &Condition::SetFilter)
        .addFunction("SetName", &Condition::SetName)
    );
    state["Filter"].setClass(kaguya::UserdataMetatable<Filter>()
        .addFunction("SetName", &Filter::SetName)
    );

    state["Loader"].setClass(kaguya::UserdataMetatable<Loader>()
        .addOverloadedFunctions("CreateNode", &Loader::CreateNode, &Loader::CreateNodeWidthArgs)
        .addOverloadedFunctions("CreateCondition", &Loader::CreateCondition, &Loader::CreateConditionWidthArgs)
        .addOverloadedFunctions("CreateFilter", &Loader::CreateFilter, &Loader::CreateFilterWidthArgs)
        .addFunction("CreateTree", &Loader::CreateTree)
    );
    state["BevaviorCache"].setClass(kaguya::UserdataMetatable<BevaviorCache>()
        .addFunction("Add", &BevaviorCache::Add)
        .addFunction("Remove", &BevaviorCache::Remove)
        .addFunction("Get", &BevaviorCache::Get)
    );
}

std::shared_ptr<Node> Loader::CreateNode(const std::string& type)
{
    auto result = registry_.CreateNode(type, { });
    if (!result)
        LoadError("Node type " + type + " not found");
    return result;
}

std::shared_ptr<Condition> Loader::CreateCondition(const std::string& type)
{
    auto result = registry_.CreateCondition(type, { });
    if (!result)
        LoadError("Condition type " + type + " not found");
    return result;
}

std::shared_ptr<Filter> Loader::CreateFilter(const std::string& type)
{
    auto result = registry_.CreateFilter(type, { });
    if (!result)
        LoadError("Filter type " + type + " not found");
    return result;
}

std::shared_ptr<Node> Loader::CreateNodeWidthArgs(const std::string& type, const ArgumentsType& arguments)
{
    auto result = registry_.CreateNode(type, arguments);
    if (!result)
        LoadError("Node type " + type + " not found");
    return result;
}

std::shared_ptr<Condition> Loader::CreateConditionWidthArgs(const std::string& type, const ArgumentsType& arguments)
{
    auto result = registry_.CreateCondition(type, arguments);
    if (!result)
        LoadError("Condition type " + type + " not found");
    return result;
}

std::shared_ptr<Filter> Loader::CreateFilterWidthArgs(const std::string& type, const ArgumentsType& arguments)
{
    auto result = registry_.CreateFilter(type, arguments);
    if (!result)
        LoadError("Filter type " + type + " not found");
    return result;
}

std::shared_ptr<Root> Loader::CreateTree(const std::string& name, const std::string& filename)
{
    auto result = LoadFile(filename);
    if (result)
        result->SetName(name);
    return result;
}

std::shared_ptr<Root> Loader::LoadString(const std::string& value)
{
    kaguya::State luaState;
    RegisterLua(luaState);
    luaState["self"] = this;
    std::shared_ptr<Root> result = std::make_shared<Root>();

    if (!luaState.dostring(value))
        return std::shared_ptr<Root>();

    luaState["init"](result);

    return result;
}

bool Loader::InitChache(const std::string& initScript, BevaviorCache& cache)
{
    kaguya::State luaState;
    RegisterLua(luaState);
    luaState["self"] = this;
    luaState["cache"] = &cache;
    if (!ExecuteScript(luaState, initScript))
        return false;
    luaState["init"]();
    return true;
}

std::shared_ptr<Root> Loader::LoadFile(const std::string& fileName)
{
    kaguya::State luaState;
    RegisterLua(luaState);
    luaState["self"] = this;
    std::shared_ptr<Root> result = std::make_shared<Root>();

    if (!ExecuteScript(luaState, fileName))
        return std::shared_ptr<Root>();

    // Call the init() function which passes in the root node
    luaState["init"](result);

    return result;
}

}
