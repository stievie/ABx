#pragma once

#include <vector>
#include <memory>
#include <stdint.h>
#include <string>
#include "Condition.h"
#include <limits>
#include "AiDefines.h"
#include "Factory.h"
#include <sa/IdGenerator.h>

namespace AI {

class Node;
typedef std::vector<std::shared_ptr<Node>> Nodes;

using NodeFactory = AbstractFactory<Node>;

#define NODE_FACTORY(NodeName)                                                              \
    class Factory : public NodeFactory                                                      \
    {                                                                                       \
    public:                                                                                 \
        std::shared_ptr<Node> Create(const ArgumentsType& arguments) const override         \
        {                                                                                   \
            std::shared_ptr<Node> res = std::make_shared<NodeName>(arguments);              \
            res->SetType(ABAI_STRINGIFY(NodeName));                                         \
            return res;                                                                     \
        }                                                                                   \
    };                                                                                      \
    static const Factory& GetFactory()                                                      \
    {                                                                                       \
        static Factory sFactory;                                                            \
        return sFactory;                                                                    \
    }

class Node
{
private:
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
    Id id_;
    std::string type_;
    std::string name_;
    Status status_{ Status::Unknown };
    std::shared_ptr<Condition> condition_;
    Status ReturnStatus(Status status)
    {
        status_ = status;
        return status_;
    }
    virtual void Initialize() {}
public:
    explicit Node(const ArgumentsType& arguments);
    virtual ~Node();
    Id GetId() const { return id_; }

    const std::string& GetType() const { return type_; }
    void SetType(const std::string& value) { type_ = value; }
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& value) { name_ = value; }
    virtual bool AddNode(std::shared_ptr<Node> node);
    void SetCondition(std::shared_ptr<Condition> condition);
    virtual Node::Status Execute(Agent& agent, uint32_t timeElapsed);
};

}
