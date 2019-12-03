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

#undef DEBUG_AI

namespace AI {
namespace Actions {

bool MoveOutAOE::TryMove(Game::Npc& npc, Game::AreaOfEffect& damager, Math::Vector3& destination)
{
    auto* rnd = GetSubsystem<Crypto::Random>();
    // Away from damager
    float angle = (npc.transformation_.position_.AngleY(damager.transformation_.position_) + Math::M_PIHALF) +
        rnd->Get<float>(-(Math::M_PIHALF / 3.0f), Math::M_PIHALF / 3.0f);

    Math::Quaternion rot(0.0f, angle, 0.0f);
    destination = Math::GetPosFromDirectionDistance(npc.transformation_.position_, rot,
        Game::RangeDistances[static_cast<size_t>(damager.GetRange())] + rnd->Get<float>(0.1f, 0.7f));
    npc.GetGame()->map_->UpdatePointHeight(destination);
#ifdef DEBUG_AI
    LOG_DEBUG << npc.GetName() << " moving from " << npc.GetPosition().ToString() << " to " << destination.ToString() << std::endl;
#endif
    return npc.autorunComp_->Goto(destination);
}

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
                if (npc.autorunComp_->IsAutoRun())
                    return Status::Running;
                return Status::Failed;
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
    {
#ifdef DEBUG_AI
        LOG_DEBUG << "No AOE damager" << std::endl;
#endif
        return Status::Failed;
    }

    Math::Vector3 destination;
    bool success = false;
    for (unsigned i = 0; i < 6; ++i)
    {
        success = TryMove(npc, *damager, destination);
        if (success)
            break;
    }

    if (success)
    {
        if (!destination.Equals(npc.transformation_.position_, Game::AT_POSITION_THRESHOLD))
        {
            if (npc.GetSpeed() < 1.0f)
                npc.SetSpeed(1.0f);
            npc.autorunComp_->SetAutoRun(true);
            npc.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
            aiAgent.aiContext_.Set(id_, destination);
            return Status::Running;
        }
        else
            return Status::Finished;
    }
#ifdef DEBUG_AI
    else
        LOG_DEBUG << "No path found from " << npc.GetPosition().ToString() << " to " << destination.ToString() << std::endl;
#endif
    return Status::Failed;
}

}
}
