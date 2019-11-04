#pragma once

#include <vector>
#include <memory>
#include <stdint.h>
#include <string>
#include "Condition.h"
#include <limits>
#include "AiTypes.h"
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
            return std::make_shared<NodeName>(arguments);                                   \
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
    static sa::IdGenerator<Id> sIds_;
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

    virtual bool AddNode(std::shared_ptr<Node> node);
    void SetCondition(std::shared_ptr<Condition> condition);
    virtual Node::Status Execute(Agent& agent, uint32_t timeElapsed);
};

}
