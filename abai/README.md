# abai

Behavior tree implementation inspired by [SimpleAI](https://github.com/mgerhardy/simpleai)
but it's single threaded and simpler.

[Here](https://outforafight.wordpress.com/2014/07/15/behaviour-behavior-trees-for-ai-dudes-part-1/)
is a nice writeup about behavior trees.

## Dependencies

* [Lua 5.3](https://www.lua.org/)
* [Kaguya](https://github.com/satoren/kaguya)
* [Some headers](../Include/sa)

## Features/Limitations

Once a BT is created it must not be modified. Nodes do not store any state
data, so it is safe to reuse BTs for other NPCs. The Agent object is used 
to store state data when necessary.

## Integration

You may need to subclass the following classes:

* `Loader`
* `Registry` to register your own Filters, Conditions and Actions
* `Agent`
* Some `Filter`s
* Some `Condition`s
* Some `Action`s

Your game object which should act somehow " intelligent" should have am `Agent`
member and in each game update `Agent::Update()` should be called with the time
passed since the last call of the `Update()` method on milliseconds. Since most
game engines uses seconds as floats you must multiply this value by 1000, then
you can pass it to the `Update()` method.

### Filter

Filters are used to select other game objects, e.g. foes to do something with
them. To create your own filter create a subclass of `AI::Filter` and override
the `Execute()` method:

Sample to select all in aggro range:

Header file:

~~~cpp
#pragma once

#include "Filter.h"

class SelectAggro : public AI::Filter
{
public:
    FILTER_FACTORY(SelectAggro)
    explicit SelectAggro(const ArgumentsType& arguments) :
        Filter(arguments)
    {}
    void Execute(Agent& agent) override;
};
~~~

CPP file:

~~~cpp
void SelectAggro::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    Game::Npc& chr = GetNpc(agent);
    // This, of course, depends on your game
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor* o)
    {
        entities.push_back(o->id_);
    });
    // Remove duplicates
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
}
~~~

### Conditions

If a node has a condition, this condition must evaluate to `true`, otherwise the
node is not executed but returns `Status::CanNotExecute`.

A special condition is the `FilterCondition` which returns `true` when the assigned
filter selected something.

### Actions

Actions are nodes that do something (aka. Leaf), e.g. attack a target. But attacking
a target makes only sense when there is a target, so an attack target action would
most likely have something like a `IsThereATarget` condition.

To create a custom action override the `DoAction()` method. Here is some code that
may demonstrate it.

Header file:

~~~cpp
#pragma once

#include "Action.h"

class AttackSelection : public AI::Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(AttackSelection)
    explicit AttackSelection(const ArgumentsType& arguments) :
        Action(arguments)
    {}
};
~~~

CPP file:

~~~cpp
Node::Status AttackSelection::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);

    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;
    for (auto id : selection)
    {
        auto target = npc.GetGame()->GetObjectById(id);
        if (!target)
            continue;
        if (!target->IsActorType())
            continue;

        if (target->IsDead())
            return Status::Finished;
        if (npc.IsAttacking(target))
            return Status::Running;
        if (npc.Attack(target))
            return Status::Running;
    }
    return Status::Finished;
}
~~~

### Register your own classes

Create a subclass of `AI::Registry` and override `Initialize()`.

~~~cpp
void MyRegistry::Initialize()
{
    Registry::Initialize();
    RegisterNodeFactory("Die", Die::GetFactory());
    // ...
    RegisterConditionFactory("IsAllyHealthLow", IsAllyHealthLow::GetFactory());
    // ...
    RegisterFilterFactory("SelectAggro", SelectAggro::GetFactory());
    // ...
}
~~~

Note: The `Initialize()` must be explicitly called by your game, e.g. when your
game starts.

### Trees

The trees itself are build with Lua, hence the dependency on [Kaguya](https://github.com/satoren/kaguya)
and [Lua](https://www.lua.org/). The loader calls the `init()` Lua function in 
the tree script and passes the root node to it, so you could do something like this:

~~~lua
function init(root)
  local prio = self:CreateNode("Priority")
  prio:AddNode(stayAlive())
  prio:AddNode(avoidDamage())
  prio:AddNode(idle(1000))
  root:AddNode(prio)
end
~~~

* `self:CreateNode("type")` creates a new node of type `type`.
* `self:CreateCondition("type")` creates a new condition of type `type`.
* `self:CreateFilter("type")` creates a new filter of type `type`.
