#include "stdafx.h"
#include "AiCharacter.h"
#include "Vector3.h"
#include "Npc.h"
#include "Map.h"

namespace AI {

AiCharacter::AiCharacter(Game::Npc& owner, const Game::Map* map) :
    Super(owner.id_),
    owner_(owner),
    map_(map)
{
    const Math::Vector3& pos = owner.transformation_.position_;
    setPosition(glm::vec3(pos.x_, pos.y_, pos.z_));
    setOrientation(owner.transformation_.rotation_);
    setSpeed(owner.GetSpeed());
    std::stringstream ss;
    ss << owner.GetName() << " " << owner.id_;
    setAttribute(ai::attributes::NAME, ss.str());
    setAttribute(ai::attributes::ID, std::to_string(owner.id_));
}

void AiCharacter::update(int64_t deltaTime, bool debuggingActive)
{
    Super::update(deltaTime, debuggingActive);
}

void AiCharacter::setPosition(const glm::vec3& position)
{
    Super::setPosition(position);
    owner_.transformation_.position_ = Math::Vector3(position.x, position.y, position.z);
}

void AiCharacter::setOrientation(float orientation)
{
    Super::setOrientation(-orientation);
    owner_.transformation_.rotation_ = orientation;
}

void AiCharacter::setSpeed(float speed)
{
    Super::setSpeed(speed);
    owner_.SetSpeed(speed);
}

}
