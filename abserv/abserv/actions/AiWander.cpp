#include "stdafx.h"
#include "AiWander.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "Logger.h"
#include "../Crowd.h"
#include "Subsystems.h"
#include "Random.h"

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

    Game::Crowd* crowd = npc.GetCrowd();

    if (crowd && crowd->GetLeader() && crowd->GetLeader() != &npc)
    {
        float distance;
        auto& aia = GetAgent(agent);
        if (aia.aiContext_.Has<distance_type>(id_))
            distance = aia.aiContext_.Get<distance_type>(id_);
        else
        {
            auto* rnd = GetSubsystem<Crypto::Random>();
            distance = Game::AT_POSITION_THRESHOLD + rnd->Get<float>(-1.0f, 1.0f);
            aia.aiContext_.Set<distance_type>(id_, distance);
        }

        // If we are in a crowd and we are not the leader, follow the leader
        Game::Npc* leader = crowd->GetLeader();
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
