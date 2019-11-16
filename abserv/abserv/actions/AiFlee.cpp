#include "stdafx.h"
#include "AiFlee.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "Subsystems.h"
#include "Random.h"
#include "../Mechanic.h"
#include "Matrix4.h"
#include "../Game.h"
#include "VectorMath.h"

namespace AI {
namespace Actions {

Node::Status Flee::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (!destination_.Equals(Math::Vector3::Zero) && agent.context_.IsActionRunning(GetId()))
    {
        if (destination_.Distance(npc.transformation_.position_) > 0.5f)
            return Status::Running;
        return Status::Finished;
    }

    // See who is damaging us and run away from that position
    auto damager = npc.damageComp_->GetLastMeleeDamager();
    if (!damager)
        return Status::Failed;

    auto* rnd = GetSubsystem<Crypto::Random>();
    // Away from damager
    float angle = (npc.transformation_.position_.AngleY(damager->transformation_.position_) + Math::M_PIHALF) +
        rnd->Get<float>(-Math::M_PIHALF / 3.0f, Math::M_PIHALF / 3.0f);

    Math::Quaternion rot(0.0f, angle, 0.0f);
    destination_ = Math::GetPosFromDirectionDistance(npc.transformation_.position_, rot,
        Game::RangeDistances[static_cast<size_t>(Game::Ranges::Adjecent)] + rnd->Get<float>(0.1f, 0.7f));
    destination_.y_ = npc.GetGame()->map_->GetTerrainHeight(destination_);

    if (npc.autorunComp_->Goto(destination_))
        return Status::Running;
    return Status::Failed;
}

}
}
