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

//#define DEBUG_AI

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
            if (npc.attackComp_->IsAttackState())
                npc.attackComp_->Cancel();
            npc.autorunComp_->SetAutoRun(true);
            npc.stateComp_.SetState(AB::GameProtocol::CreatureState::Moving);
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
