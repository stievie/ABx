#pragma once

#include <vector>
#include <memory>
#include <stdint.h>
#include <string>
#include "Condition.h"
#include <limits>
#include "Id.h"
#include "Factory.h"
#include <sa/IdGenerator.h>

namespace AI {

class Node;
typedef std::vector<std::shared_ptr<Node>> Nodes;

struct NodeFactoryContext
{
    std::shared_ptr<Condition> condition;
    union {
        uint32_t millis;
        uint32_t times;
    };
};

using NodeFactory = AbstractFactory<Node, NodeFactoryContext>;

#define NODE_FACTORY(NodeName) \
	class Factory : public NodeFactory                                                      \
    {                                                                                       \
	public:                                                                                 \
		std::shared_ptr<Node> Create(const NodeFactoryContext& ctx) const override          \
        {                                                                                   \
			return std::make_shared<NodeName>(ctx);                                         \
		}                                                                                   \
	};                                                                                      \
	static const Factory& GetFactory() {                                                    \
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
    std::shared_ptr<Condition> condition_;
    Id id_;
public:
    explicit Node(const NodeFactoryContext& ctx);
    virtual ~Node();

    Id GetId() const { return id_; }

    virtual bool AddChild(std::shared_ptr<Node> node);
    void SetCondition(std::shared_ptr<Condition> condition);
    virtual void Initialize() {};
    virtual Node::Status Execute(Agent& agent, uint32_t timeElapsed);
};

}
