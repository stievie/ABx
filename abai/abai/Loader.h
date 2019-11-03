#pragma once

#include "Registry.h"
#include <memory>
#include <string>
#include <kaguya/kaguya.hpp>

namespace AI {

class Node;
class Root;
class Condition;
class Filter;

class Loader
{
private:
    Registry& registry_;
    static void RegisterLua(kaguya::State& state);;
    std::shared_ptr<Node> AddNode(Node* parent, const std::string& type);
    std::shared_ptr<Condition> CreateCondition(const std::string& type);
    std::shared_ptr<Filter> CreateFilter(const std::string& type);
public:
    explicit Loader(Registry& reg);
    std::shared_ptr<Root> Load(const std::string& fileName);
};

}
