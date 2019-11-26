#include "stdafx.h"
#include "AiMoveOutAOE.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "Subsystems.h"
#include "Random.h"
#include "../Mechanic.h"
#include "Matrix4.h"
#include "../Game.h"
#include "../AreaOfEffect.h"
#include "VectorMath.h"

namespace AI {
namespace Actions {

Node::Status MoveOutAOE::DoAction(Agent& agent, uint32_t)
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

    Game::AreaOfEffect* damager = nullptr;
    npc.VisitInRange(Game::Ranges::Aggro, [&](const Game::GameObject& object)
    {
        if (Game::Is<Game::AreaOfEffect>(object))
        {
            const auto& aoe = Game::To<Game::AreaOfEffect>(object);
            if (aoe.IsEnemy(&npc) && aoe.HasEffect(Game::SkillEffectDamage) && aoe.IsInRange(&npc))
            {
                damager = const_cast<Game::AreaOfEffect*>(&aoe);
                return Iteration::Break;
            }
        }
        return Iteration::Continue;
    });

    if (!damager)
        return Status::Failed;

    auto* rnd = GetSubsystem<Crypto::Random>();
    // Away from damager
    float angle = (npc.transformation_.position_.AngleY(damager->transformation_.position_) + Math::M_PIHALF) +
        rnd->Get<float>(-Math::M_PIHALF / 3.0f, Math::M_PIHALF / 3.0f);

    Math::Quaternion rot(0.0f, angle, 0.0f);
    Math::Vector3 destination = Math::GetPosFromDirectionDistance(npc.transformation_.position_, rot,
        Game::RangeDistances[static_cast<size_t>(damager->GetRange())] + rnd->Get<float>(0.1f, 0.7f));
    npc.GetGame()->map_->UpdatePointHeight(destination);

    if (npc.autorunComp_->Goto(destination))
    {
        if (npc.GetSpeed() < 1.0f)
            npc.SetSpeed(1.0f);
        aiAgent.aiContext_.Set(id_, destination);
        return Status::Running;
    }
    return Status::Failed;
}

}
}
