/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "AiWander.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "../Group.h"

//#define DEBUG_AI

namespace AI {
namespace Actions {

Node::Status Wander::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (!npc.IsWander())
    {
#ifdef DEBUG_AI
        LOG_WARNING << npc.GetName() << " has no wander component" << std::endl;
#endif
        return Status::Failed;
    }
    if (npc.IsImmobilized())
        return Status::Failed;

    Game::Group* group = npc.GetGroup();

    if (group && group->GetLeader() && group->GetLeader() != &npc)
    {
        float distance;
        auto& aia = GetAgent(agent);
        if (aia.aiContext_.Has<DistanceType>(id_))
            distance = aia.aiContext_.Get<DistanceType>(id_);
        else
        {
            auto* rnd = GetSubsystem<Crypto::Random>();
            distance = Game::AT_POSITION_THRESHOLD + rnd->Get<float>(-1.0f, 1.0f);
            aia.aiContext_.Set<DistanceType>(id_, distance);
        }

        // If we are in a crowd and we are not the leader, follow the leader
        Game::Actor* leader = group->GetLeader();
        if (npc.GetPosition().Equals(leader->GetPosition(), distance))
            return Status::Finished;
        if (npc.autorunComp_->IsFollowing(*leader))
            return Status::Running;
        npc.SetSpeed(leader->GetSpeed());
        npc.autorunComp_->Follow(leader->GetPtr<Game::GameObject>(), false, distance);
        return Status::Running;
    }

    // Not in a crowd or we are the leader and our fellow witches follow us
    if (npc.wanderComp_->IsWandering())
        return Status::Running;

    if (!npc.wanderComp_->Wander(true))
    {
#ifdef DEBUG_AI
        LOG_DEBUG << npc.GetName() << " failed wandering" << std::endl;
#endif
        return Status::Failed;
    }

    if (npc.GetSpeed() > 0.5f)
        npc.SetSpeed(0.5f);

#ifdef DEBUG_AI
    LOG_DEBUG << npc.GetName() << " start wandering" << std::endl;
#endif
    return Status::Running;
}

Wander::Wander(const ArgumentsType& arguments) :
    Action(arguments)
{ }

}
}
