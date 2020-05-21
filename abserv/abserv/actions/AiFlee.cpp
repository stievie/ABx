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
#include "AiFlee.h"
#include "../AiAgent.h"
#include "../DamageComp.h"
#include "../Game.h"
#include "../Npc.h"
#include <abshared/Mechanic.h>
#include <absmath/Matrix4.h>
#include <absmath/Vector3.h>
#include <absmath/VectorMath.h>

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
