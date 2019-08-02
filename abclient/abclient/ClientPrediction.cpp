#include "stdafx.h"
#include "ClientPrediction.h"
#include "Player.h"
#include "MathUtils.h"
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include "LevelManager.h"

void ClientPrediction::RegisterObject(Context* context)
{
    context->RegisterFactory<ClientPrediction>();
}

ClientPrediction::ClientPrediction(Context* context) :
    LogicComponent(context),
    serverTime_(0)
{
    serverPos_.y_ = std::numeric_limits<float>::max();
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

ClientPrediction::~ClientPrediction() = default;

void ClientPrediction::UpdateMove(float timeStep, uint8_t direction, float speedFactor)
{
    if (direction == 0)
        return;

    const float speed = GetSpeed(timeStep, Game::BASE_MOVE_SPEED, speedFactor);
    if ((direction & AB::GameProtocol::MoveDirectionNorth) == AB::GameProtocol::MoveDirectionNorth)
        Move(speed, Vector3::FORWARD);
    if ((direction & AB::GameProtocol::MoveDirectionSouth) == AB::GameProtocol::MoveDirectionSouth)
        // Move slower backward
        Move(speed, Vector3::BACK / 2.0f);
    if ((direction & AB::GameProtocol::MoveDirectionWest) == AB::GameProtocol::MoveDirectionWest)
        Move(speed, Vector3::LEFT / 2.0f);
    if ((direction & AB::GameProtocol::MoveDirectionEast) == AB::GameProtocol::MoveDirectionEast)
        Move(speed, Vector3::RIGHT / 2.0f);
}

bool ClientPrediction::CheckCollision(const Vector3& pos)
{
    PhysicsWorld* physWorld = GetScene()->GetComponent<PhysicsWorld>();
    if (!physWorld)
        return true;
    CollisionShape* collShape = node_->GetComponent<CollisionShape>(true);
    if (!collShape)
        return true;

    LevelManager* lMan = GetSubsystem<LevelManager>();
    const bool isCollidingWithPlayers = lMan->GetMapType() > AB::Entities::GameTypeOutpost;

    // Actors always have AABB
    const BoundingBox bb = collShape->GetWorldBoundingBox();
    const Vector3 half = bb.HalfSize();
    const BoundingBox newBB(pos - half, pos + half);
    PODVector<RigidBody*> result;
    physWorld->GetRigidBodies(result, newBB);
    if (result.Size() < 2)
        return true;

    for (auto i = result.Begin(); i != result.End(); ++i)
    {
        if (!(*i)->GetNode())
            continue;

        Node* node = (*i)->GetNode()->GetParent();
        if (!node)
            continue;
        if (node->GetComponent<Player>() != nullptr)
            continue;

        auto actor = node->GetComponent<Actor>();
        if (!actor)
        {
            // Always collide with static objects
            return false;
        }

        const ObjectType type = actor->objectType_;
        if (type == ObjectTypeSelf)
            // Don't collide with self
            continue;
        else if (type == ObjectTypePlayer && !isCollidingWithPlayers)
            // Don't collide with other players in outposts
            continue;
        else if (type == ObjectTypeAreaOfEffect || type == ObjectTypeItemDrop)
            // Never collide with these
            continue;
        // When we are here we do collide with some object and can't go to pos
        return false;
    }
    return true;
}

void ClientPrediction::Move(float speed, const Vector3& amount)
{
    Player* player = node_->GetComponent<Player>();
    const Quaternion& oriention = player->rotateTo_;
    Vector3 pos = player->moveToPos_;
    const Matrix3 m = oriention.RotationMatrix();
    const Vector3 a = amount * speed;
    const Vector3 v = m * a;
    pos += v;
    Terrain* terrain = GetScene()->GetComponent<Terrain>(true);
    if (terrain)
        pos.y_ = terrain->GetHeight(pos);
    else
    {
        if (!Equals(serverPos_.y_, std::numeric_limits<float>::max()))
            pos.y_ = serverPos_.y_;
    }

    if (CheckCollision(pos))
        player->moveToPos_ = pos;
}

void ClientPrediction::UpdateTurn(float timeStep, uint8_t direction, float speedFactor)
{
    if (direction == 0)
        return;

    const float speed = GetSpeed(timeStep, Game::BASE_TURN_SPEED, speedFactor);
    if ((direction & AB::GameProtocol::TurnDirectionLeft) == AB::GameProtocol::TurnDirectionLeft)
        Turn(speed);
    if ((direction & AB::GameProtocol::TurnDirectionRight) == AB::GameProtocol::TurnDirectionRight)
        Turn(-speed);
}

void ClientPrediction::Turn(float yAngle)
{
    float deg = RadToDeg(yAngle);
    NormalizeAngle(deg);
    Player* player = node_->GetComponent<Player>();
    const float newangle = player->rotateTo_.EulerAngles().y_ + deg;
    TurnAbsolute(newangle);
}

void ClientPrediction::TurnAbsolute(float yAngle)
{
    Player* player = node_->GetComponent<Player>();
    player->rotateTo_.FromEulerAngles(0.0f, yAngle, 0.0f);
}

void ClientPrediction::FixedUpdate(float timeStep)
{
    LogicComponent::FixedUpdate(timeStep);

    extern bool gNoClientPrediction;
    if (gNoClientPrediction)
        return;

    Player* player = node_->GetComponent<Player>();

    const AB::GameProtocol::CreatureState state = player->GetCreatureState();
    if (state != AB::GameProtocol::CreatureStateIdle && state != AB::GameProtocol::CreatureStateMoving)
        return;

    const uint8_t moveDir = player->GetMoveDir();
    if (moveDir != 0)
    {
        if (state == AB::GameProtocol::CreatureStateIdle)
        {
            TurnAbsolute(player->controls_.yaw_);
            player->SetCreatureState(serverTime_, AB::GameProtocol::CreatureStateMoving);
        }
        UpdateMove(timeStep, moveDir, player->GetSpeedFactor());
    }

    const uint8_t turnDir = player->GetTurnDir();
    if (turnDir != 0)
    {
        if (state == AB::GameProtocol::CreatureStateIdle)
            player->SetCreatureState(serverTime_, AB::GameProtocol::CreatureStateMoving);
        UpdateTurn(timeStep, turnDir, player->GetSpeedFactor());
    }

    if (state == AB::GameProtocol::CreatureStateMoving)
    {
        if ((moveDir == 0 && lastMoveDir_ != 0) ||
            (turnDir == 0 && lastTurnDir_ != 0))
            player->SetCreatureState(serverTime_, AB::GameProtocol::CreatureStateIdle);
    }

    lastMoveDir_ = moveDir;
    lastTurnDir_ = turnDir;
}

void ClientPrediction::CheckServerPosition(int64_t time, const Vector3& serverPos)
{
    serverTime_ = time;
    this->serverPos_ = serverPos;
    Player* player = node_->GetComponent<Player>();
    const Vector3& currPos = player->moveToPos_;
    const float dist = (fabs(currPos.x_ - serverPos.x_) + fabs(currPos.z_ - serverPos.z_)) * 0.5f;
    // FIXME: This sucks a bit, and needs some work.
    if (dist > 5.0f || (dist > 1.0f && player->GetCreatureState() == AB::GameProtocol::CreatureStateIdle))
    {
        // If too far away or player is idle, Lerp to server position
        player->moveToPos_ = serverPos;
    }
}

void ClientPrediction::CheckServerRotation(int64_t time, float rad)
{
    serverTime_ = time;
    Player* player = node_->GetComponent<Player>();
    const Quaternion& currRot = player->rotateTo_;
    float deg = RadToDeg(rad);
    NormalizeAngle(deg);
    if (fabs(currRot.EulerAngles().y_ - deg) > 1.0f)
    {
        player->rotateTo_.FromEulerAngles(0.0f, deg, 0.0f);
    }
}
