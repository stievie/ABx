#pragma once

#include "Registry.h"
#include <memory>
#include <string>
#include <map>

namespace AI {

class Node;
class Root;
class Condition;
class Filter;

class Loader
{
private:
    Registry& registry_;
    void RegisterLua(kaguya::State& state);
    std::shared_ptr<Node> CreateNode(const std::string& type);
    std::shared_ptr<Condition> CreateCondition(const std::string& type);
    std::shared_ptr<Filter> CreateFilter(const std::string& type);
    std::shared_ptr<Node> CreateNodeWidthArgs(const std::string& type, const ArgumentsType& arguments);
    std::shared_ptr<Condition> CreateConditionWidthArgs(const std::string& type, const ArgumentsType& arguments);
    std::shared_ptr<Filter> CreateFilterWidthArgs(const std::string& type, const ArgumentsType& arguments);
    // Subclasses should override it to get the full filename of an include file.
    virtual std::string GetScriptFile(const std::string file) { return file; }
protected:
    // Can be overwritten if the script is somehow cached
    virtual bool ExecuteScript(kaguya::State& state, const std::string& file);
    virtual void LoadError(const std::string&) { }
public:
    explicit Loader(Registry& reg);
    virtual ~Loader();
    std::shared_ptr<Root> LoadFile(const std::string& fileName);
    std::shared_ptr<Root> LoadString(const std::string& value);
};

}
