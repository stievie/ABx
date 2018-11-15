#include "stdafx.h"
#include "AiCharacter.h"
#include "Vector3.h"
#include "Npc.h"
#include "Map.h"
#include "Game.h"

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
    auto pos = Math::Vector3(position.x, position.y, position.z);
    pos.y_ = owner_.GetGame()->map_->GetTerrainHeight(pos);
    Super::setPosition(glm::vec3(pos.x_, pos.y_, pos.z_));
//    owner_.transformation_.position_ = pos;
//    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
//    owner_.moveComp_.moved_ = true;
    owner_.GotoPosition(pos);
    {
//        LOG_INFO << owner_.GetName() << " going to " << pos.ToString() << std::endl;
//        owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
//        owner_.autorunComp_.autoRun_ = true;
    }
}

void AiCharacter::setOrientation(float orientation)
{
    Super::setOrientation(orientation);
    owner_.moveComp_.SetDirection(orientation);
}

void AiCharacter::setSpeed(float speed)
{
    owner_.SetSpeed(speed);
    Super::setSpeed(speed);
}

}
