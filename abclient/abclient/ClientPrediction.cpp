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

#include "ClientPrediction.h"
#include "Player.h"
#include "MathUtils.h"
#include "HeightMap.h"
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include "LevelManager.h"
#include "FwClient.h"
#include <sa/time.h>

void ClientPrediction::RegisterObject(Context* context)
{
    context->RegisterFactory<ClientPrediction>();
}

ClientPrediction::ClientPrediction(Context* context) :
    LogicComponent(context),
    serverTime_(0),
    lastStateChange_(sa::time::tick())
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
        Move(speed * Game::MOVE_BACK_FACTOR, Vector3::BACK);
    if ((direction & AB::GameProtocol::MoveDirectionWest) == AB::GameProtocol::MoveDirectionWest)
        Move(speed * Game::MOVE_BACK_FACTOR, Vector3::LEFT);
    if ((direction & AB::GameProtocol::MoveDirectionEast) == AB::GameProtocol::MoveDirectionEast)
        Move(speed * Game::MOVE_BACK_FACTOR, Vector3::RIGHT);
}

bool ClientPrediction::CheckCollision(const Vector3& pos)
{
    PhysicsWorld* physWorld = GetScene()->GetComponent<PhysicsWorld>();
    if (!physWorld)
        return true;
    CollisionShape* collShape = node_->GetComponent<CollisionShape>(true);
    if (!collShape)
        return true;
    RigidBody* rigidbody = node_->GetComponent<RigidBody>(true);
    if (!rigidbody)
        return true;

    LevelManager* lMan = GetSubsystem<LevelManager>();
    const bool isCollidingWithPlayers = !AB::Entities::IsOutpost(lMan->GetMapType());

    // Actors always have AABB
    const BoundingBox bb = collShape->GetWorldBoundingBox();
    const Vector3 half = bb.HalfSize();
    const BoundingBox newBB(pos - half, pos + half);
    PODVector<RigidBody*> result;
    physWorld->GetRigidBodies(result, newBB, rigidbody->GetCollisionMask());
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
            // Thats we
            continue;

        auto actor = node->GetComponent<Actor>();
        if (!actor)
        {
            // Always collide with static objects on the same collision layer
            auto* otherRb = (*i)->GetNode()->GetComponent<RigidBody>(true);
            if (!otherRb)
                return true;
            return true;
        }

        const ObjectType type = actor->objectType_;
        if (type == ObjectType::Self)
            // Don't collide with self
            continue;
        else if (type == ObjectType::Player && !isCollidingWithPlayers)
            // Don't collide with other players in outposts
            continue;
        else if (type == ObjectType::AreaOfEffect || type == ObjectType::ItemDrop)
            // Never collide with these
            continue;
        else if ((type == ObjectType::Player || type == ObjectType::Npc) && actor->IsDead())
            // Dont collide with dead actors
            continue;
        // When we are here we do collide with some object and can't go to pos
        return false;
    }
    return true;
}

Terrain* ClientPrediction::GetTerrain() const
{
    if (!terrain_)
        terrain_ = GetScene()->GetComponent<Terrain>(true);
    return terrain_.Get();
}

HeightMap* ClientPrediction::GetHeightMap() const
{
    if (!heightMap_)
        heightMap_ = GetScene()->GetComponent<HeightMap>();
    return heightMap_.Get();

}

float ClientPrediction::GetHeight(const Vector3& world) const
{
    Terrain* terrain = GetTerrain();
    if (!terrain)
        return 0.0f;

    float result = terrain->GetHeight(world);
    HeightMap* hm = GetHeightMap();
    if (!hm)
        return result;
    float result2 = hm->GetHeight(world);

    if (IsInf(result2) || result2 < result)
        return result;

//    URHO3D_LOGINFOF("GetHeight() height1 %f, height2 %f", result, result2);

    // If the difference is smaller than the height of the character it can't be the lower height
    float diff12 = result2 - result;
    if (diff12 < 1.7f)
        return result2;

    // Otherwise use the closer value to the current height
    if (fabs(world.y_ - result) < fabs(world.y_ - result2))
        return result;
    return result2;
}

void ClientPrediction::Move(float speed, const Vector3& amount)
{
    Player* player = node_->GetComponent<Player>();
    const Quaternion& oriention = player->GetRotateTo();
    Vector3 pos = player->GetMoveToPos();
    const Matrix3 m = oriention.RotationMatrix();
    const Vector3 a = amount * speed;
    const Vector3 v = m * a;
    pos += v;

    float height = GetHeight(pos);
    if (!Equals(height, 0.0f))
        pos.y_ = height;
//    URHO3D_LOGINFOF("Move() pos %s", pos.ToString().CString());

    if (CheckCollision(pos))
        player->SetMoveToPos(pos);
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
    const float deg = NormalizedAngle(RadToDeg(yAngle));
    Player* player = node_->GetComponent<Player>();
    const float newangle = player->GetRotateTo().YawAngle() + deg;
    TurnAbsolute(newangle);
}

void ClientPrediction::TurnAbsolute(float yAngle)
{
    Player* player = node_->GetComponent<Player>();
    player->SetRotateTo({ yAngle, Vector3::UP });
}

void ClientPrediction::FixedUpdate(float timeStep)
{
    LogicComponent::FixedUpdate(timeStep);

    extern bool gNoClientPrediction;
    if (gNoClientPrediction)
        return;

    Player* player = node_->GetComponent<Player>();

    const AB::GameProtocol::CreatureState state = player->GetCreatureState();
    if (lastState_ != state)
    {
        lastStateChange_ = sa::time::tick();
        lastState_ = state;
    }

    if (state != AB::GameProtocol::CreatureState::Idle && state != AB::GameProtocol::CreatureState::Moving)
        return;

    const uint8_t moveDir = player->GetMoveDir();
    if (moveDir != 0)
    {
        if (state == AB::GameProtocol::CreatureState::Idle)
        {
            TurnAbsolute(player->controls_.yaw_);
            player->SetCreatureState(serverTime_, AB::GameProtocol::CreatureState::Moving);
            lastStateChange_ = sa::time::tick();
        }
        UpdateMove(timeStep, moveDir, player->GetSpeedFactor());
    }

    const uint8_t turnDir = player->GetTurnDir();
    if (turnDir != 0)
    {
        if (state == AB::GameProtocol::CreatureState::Idle)
        {
            player->SetCreatureState(serverTime_, AB::GameProtocol::CreatureState::Moving);
            lastStateChange_ = sa::time::tick();
        }
        UpdateTurn(timeStep, turnDir, player->GetSpeedFactor());
    }

    if (state == AB::GameProtocol::CreatureState::Moving)
    {
        if ((moveDir == 0 && lastMoveDir_ != 0) ||
            (turnDir == 0 && lastTurnDir_ != 0))
        {
            player->SetCreatureState(serverTime_, AB::GameProtocol::CreatureState::Idle);
            lastStateChange_ = sa::time::tick();
        }
    }

    lastMoveDir_ = moveDir;
    lastTurnDir_ = turnDir;
}

void ClientPrediction::CheckServerPosition(int64_t time, const Vector3& serverPos)
{
    serverTime_ = time;
    this->serverPos_ = serverPos;

    Player* player = node_->GetComponent<Player>();
    FwClient* client = GetSubsystem<FwClient>();

    const Vector3& currPos = player->GetMoveToPos();
    const float dist = (fabs(currPos.x_ - serverPos.x_) + fabs(currPos.z_ - serverPos.z_)) * 0.5f;
    const float distThreshold = ((float)client->GetLastPing() * 3.0f) / 10000.0f;
//    URHO3D_LOGINFOF("Ping %d, Dist %f, Thres %f", client->GetLastPing(), dist, distThreshold);
    // FIXME: This sucks a bit, and needs some work.
    if (dist > distThreshold &&
        (int)sa::time::time_elapsed(lastStateChange_) > client->GetLastPing() + 50 &&
        player->GetCreatureState() == AB::GameProtocol::CreatureState::Idle)
    {
        // If too far away or player is idle, Lerp to server position
        player->SetMoveToPos(serverPos);
    }
}

void ClientPrediction::CheckServerRotation(int64_t time, float rad)
{
    serverTime_ = time;
    Player* player = node_->GetComponent<Player>();
    const Quaternion& currRot = player->GetRotateTo();
    const float deg = NormalizedAngle(RadToDeg(rad));
    if (fabs(currRot.YawAngle() - deg) > 1.0f)
    {
        player->SetRotateTo({ deg, Vector3::UP });
    }
}
