# abai

Behavior tree implementation inspired by [SimpleAI](https://github.com/mgerhardy/simpleai)
but it's single threaded and simpler.

[Here](https://outforafight.wordpress.com/2014/07/15/behaviour-behavior-trees-for-ai-dudes-part-1/)
is a nice writeup about behavior trees.

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
passed since the last call of the `Update()` method on milliseconds.

### Filter

Filters are used to select other game objects, e.g. foes to do something with
them. To create your own filter create a subclass of `AI::Filter` and override
the `Execute()` method:

Sample to select all inn aggro range:

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

If a node has a condition, this condition must evaluate to `true` otherwise the
node is not executed but returns `Status::CanNotExecute`.

A special condition is the `FilterCondition` which returns `true` when the assigned
filter selected something.

### Actions

Actions are nodes that do something (aka. Leaf), e.g. attack a target. But attacking
a target makes only sense when there is a target, so an attack target action would
most likely have something like a `IsThereATarget` condition.

