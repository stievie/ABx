#include "stdafx.h"
#include "LuaLoader.h"
#include "Root.h"
#include "Condition.h"
#include "Filter.h"
#include <sa/StringUtils.h>
#include "BevaviorCache.h"

namespace AI {

bool LuaLoader::ExecuteScript(kaguya::State& state, const std::string& file)
{
    const std::string filename = GetScriptFile(file);
    if (filename.empty())
        return false;
    return state.dofile(filename.c_str());
}

void LuaLoader::RegisterLua(kaguya::State& state)
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
    state["node"] = kaguya::overload(
        [this](const std::string& type) { return CreateNodeWidthArgs(type, { }); },
        [this](const std::string& type, const ArgumentsType& arguments) { return CreateNodeWidthArgs(type, arguments); }
    );
    state["filter"] = kaguya::overload(
        [this](const std::string& type) { return CreateFilterWidthArgs(type, { }); },
        [this](const std::string& type, const ArgumentsType& arguments) { return CreateFilterWidthArgs(type, arguments); }
    );
    state["condition"] = kaguya::overload(
        [this](const std::string& type) { return CreateConditionWidthArgs(type, { }); },
        [this](const std::string& type, const ArgumentsType& arguments) { return CreateConditionWidthArgs(type, arguments); }
    );
    state["tree"] = kaguya::function(
        [this](const std::string& name, const std::string& filename) { return CreateTree(name, filename); }
    );

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

    state["BevaviorCache"].setClass(kaguya::UserdataMetatable<BevaviorCache>()
        .addFunction("Add", &BevaviorCache::Add)
        .addFunction("Remove", &BevaviorCache::Remove)
        .addFunction("Get", &BevaviorCache::Get)
    );
}

std::shared_ptr<Node> LuaLoader::CreateNode(const std::string& type)
{
    return CreateNodeWidthArgs(type, { });
}

std::shared_ptr<Condition> LuaLoader::CreateCondition(const std::string& type)
{
    return CreateConditionWidthArgs(type, { });
}

std::shared_ptr<Filter> LuaLoader::CreateFilter(const std::string& type)
{
    return CreateFilterWidthArgs(type, { });
}

std::shared_ptr<Node> LuaLoader::CreateNodeWidthArgs(const std::string& type, const ArgumentsType& arguments)
{
    auto result = registry_.CreateNode(type, arguments);
    if (!result)
        LoadError("Node type " + type + " not found");
    return result;
}

std::shared_ptr<Condition> LuaLoader::CreateConditionWidthArgs(const std::string& type, const ArgumentsType& arguments)
{
    auto result = registry_.CreateCondition(type, arguments);
    if (!result)
        LoadError("Condition type " + type + " not found");
    return result;
}

std::shared_ptr<Filter> LuaLoader::CreateFilterWidthArgs(const std::string& type, const ArgumentsType& arguments)
{
    auto result = registry_.CreateFilter(type, arguments);
    if (!result)
        LoadError("Filter type " + type + " not found");
    return result;
}

std::shared_ptr<Root> LuaLoader::CreateTree(const std::string& name, const std::string& filename)
{
    auto result = LoadFile(filename);
    if (result)
        result->SetName(name);
    return result;
}

std::shared_ptr<Root> LuaLoader::LoadString(const std::string& value)
{
    kaguya::State luaState;
    RegisterLua(luaState);
    std::shared_ptr<Root> result = std::make_shared<Root>();

    if (!luaState.dostring(value))
        return std::shared_ptr<Root>();

    luaState["init"](result);

    return result;
}

bool LuaLoader::InitChache(const std::string& initScript, BevaviorCache& cache)
{
    kaguya::State luaState;
    RegisterLua(luaState);
    if (!ExecuteScript(luaState, initScript))
        return false;
    luaState["init"](&cache);
    return true;
}

std::shared_ptr<Root> LuaLoader::LoadFile(const std::string& fileName)
{
    kaguya::State luaState;
    RegisterLua(luaState);
    std::shared_ptr<Root> result = std::make_shared<Root>();

    if (!ExecuteScript(luaState, fileName))
        return std::shared_ptr<Root>();

    // Call the init() function which passes in the root node
    luaState["init"](result);

    return result;
}

}
