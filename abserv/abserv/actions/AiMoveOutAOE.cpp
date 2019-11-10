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
    if (!destination_.Equals(Math::Vector3::Zero) && agent.IsActionRunning(GetId()))
    {
        if (destination_.Distance(npc.transformation_.position_) > 0.5f)
            return Status::Running;
        return Status::Finished;
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
    destination_ = Math::GetPosFromDirectionDistance(npc.transformation_.position_, rot,
        Game::RangeDistances[static_cast<size_t>(damager->GetRange())] + rnd->Get<float>(0.1f, 0.7f));
    destination_.y_ = npc.GetGame()->map_->GetTerrainHeight(destination_);

    if (npc.autorunComp_->Goto(destination_))
        return Status::Running;
    return Status::Failed;
}

}
}
