#pragma once

#include <vector>
#include <memory>
#include <stdint.h>
#include <string>
#include "Condition.h"
#include "AiDefines.h"
#include "Factory.h"
#include <functional>
#include <sa/IdGenerator.h>
#include <sa/Iteration.h>

namespace AI {

class Node;
typedef std::vector<std::shared_ptr<Node>> Nodes;

using NodeFactory = AbstractFactory<Node>;

#define NODE_CLASS(NodeName)                                                        \
public:                                                                             \
    class Factory : public NodeFactory                                              \
    {                                                                               \
    public:                                                                         \
        std::shared_ptr<Node> Create(const ArgumentsType& arguments) const override \
        {                                                                           \
            std::shared_ptr<Node> res = std::make_shared<NodeName>(arguments);      \
            return res;                                                             \
        }                                                                           \
    };                                                                              \
    static const Factory& GetFactory()                                              \
    {                                                                               \
        static Factory sFactory;                                                    \
        return sFactory;                                                            \
    }                                                                               \
    NodeName(const NodeName&) = delete;                                             \
    NodeName& operator=(const NodeName&) = delete;                                  \
    NodeName(NodeName&&) = delete;                                                  \
    NodeName& operator=(NodeName&&) = delete;                                       \
    const char* GetClassName() const override { return ABAI_STRINGIFY(NodeName); }

class Node
{
private:
    // Autogenerated Node IDs
    static sa::IdGenerator<Id> sIDs;
public:
    enum class Status
    {
        Unknown,
        CanNotExecute,
        Running,
        Finished,
        Failed,

        _Count_
    };
protected:
    // Node ID managed by the library.
    Id id_;
    std::string name_;
    std::shared_ptr<Condition> condition_;
public:
    explicit Node(const ArgumentsType& arguments);
    virtual ~Node();
    Id GetId() const { return id_; }

    virtual const char* GetClassName() const = 0;
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& value) { name_ = value; }
    virtual bool AddNode(std::shared_ptr<Node> node);
    virtual void VisitChildren(const std::function<Iteration(const Node&)>&) const { }
    void SetCondition(std::shared_ptr<Condition> condition);
    const Condition* GetCondition() const { return condition_.get(); }
    virtual Node::Status Execute(Agent& agent, uint32_t timeElapsed);
};

}
