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
    auto& aiAgent = GetAgent(agent);
    if (IsCurrentAction(agent))
    {
        if (aiAgent.aiContext_.Has<Math::Vector3>(id_))
        {
            const auto& dest = aiAgent.aiContext_.Get<Math::Vector3>(id_);
            if (!dest.Equals(Math::Vector3::Zero))
            {
                if (dest.Equals(npc.transformation_.position_, Game::AT_POSITION_THRESHOLD))
                    return Status::Finished;
                return Status::Running;
            }
        }
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
    Math::Vector3 destination = Math::GetPosFromDirectionDistance(npc.transformation_.position_, rot,
        Game::RangeDistances[static_cast<size_t>(Game::Ranges::Adjecent)] + rnd->Get<float>(0.1f, 0.7f));
    npc.GetGame()->map_->UpdatePointHeight(destination);

    if (npc.autorunComp_->Goto(destination))
    {
        aiAgent.aiContext_.Set(id_, destination);
        return Status::Running;
    }
    return Status::Failed;
}

}
}
