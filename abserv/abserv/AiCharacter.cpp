#include "stdafx.h"
#include "AiCharacter.h"
#include "Vector3.h"
#include "Npc.h"
#include "Map.h"
#include "Game.h"
#include "Actor.h"

namespace AI {

AiCharacter::AiCharacter(Game::Npc& owner) :
    Super(owner.id_),
    owner_(owner)
{
    const Math::Vector3& pos = owner.transformation_.position_;
    Super::setPosition(glm::vec3(pos.x_, pos.y_, pos.z_));
    Super::setOrientation(owner.transformation_.GetYRotation());
    Super::setSpeed(owner.GetSpeed() * 2.0f);
    std::stringstream ss;
    ss << owner.GetName() << " " << owner.id_;
    setAttribute(ai::attributes::NAME, ss.str());
    setAttribute(ai::attributes::ID, std::to_string(owner.id_));
}

void AiCharacter::update(int64_t deltaTime, bool debuggingActive)
{
    ai::AggroMgr& aggro = owner_.GetAi()->getAggroMgr();
    // https://github.com/mgerhardy/engine/blob/a3ebd511c9009f8b58937599df03d86ed6efdbf1/src/modules/backend/entity/ai/AICharacter.cpp
    owner_.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor* o)
    {
        float aggroValue = owner_.GetAggro(o);
        aggro.addAggro(static_cast<int>(o->id_), aggroValue);
    });

    Super::update(deltaTime, debuggingActive);
}

void AiCharacter::setPosition(const glm::vec3& position)
{
    auto pos = Math::Vector3(position.x, position.y, position.z);
    if (owner_.moveComp_->SetPosition(pos))
    {
        owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
        Math::Vector3 realPos = owner_.GetPosition();
        Super::setPosition(glm::vec3(realPos.x_, realPos.y_, realPos.z_));
    }
}

void AiCharacter::setOrientation(float orientation)
{
    Super::setOrientation(orientation);
//    owner_.moveComp_.SetDirection(orientation);
}

void AiCharacter::setSpeed(float speed)
{
    owner_.SetSpeed(speed / 2.0f);
    Super::setSpeed(speed);
}

}
