#include "stdafx.h"
#include "ClientPrediction.h"
#include "Player.h"
#include "MathUtils.h"
#include <Urho3D/Physics/PhysicsWorld.h>

void ClientPrediction::RegisterObject(Context* context)
{
    context->RegisterFactory<ClientPrediction>();
}

ClientPrediction::ClientPrediction(Context* context) :
    LogicComponent(context),
    serverTime_(0),
    serverY_(std::numeric_limits<float>::max())
{
    SetUpdateEventMask(USE_UPDATE | USE_FIXEDUPDATE);
}

ClientPrediction::~ClientPrediction()
{
}

void ClientPrediction::UpdateMove(float timeStep, uint8_t direction, float speedFactor)
{
    const float speed = GetSpeed(timeStep, BASE_SPEED, speedFactor);
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

void ClientPrediction::Move(float speed, const Vector3& amount)
{
    Player* player = node_->GetComponent<Player>();
    Quaternion oriention = player->rotateTo_;
    Vector3 pos = player->moveToPos_;
    const Matrix3 m = oriention.RotationMatrix();
    const Vector3 a = amount * speed;
    const Vector3 v = m * a;
    pos += v;
    Terrain* terrain = node_->GetScene()->GetComponent<Terrain>(true);
    if (terrain)
        pos.y_ = terrain->GetHeight(pos);
    else
    {
        if (!Equals(serverY_, std::numeric_limits<float>::max()))
            pos.y_ = serverY_;
    }
//    URHO3D_LOGINFOF("speed %f, amount %s, v %s, new pos %s", speed, amount.ToString().CString(), v.ToString().CString(), pos.ToString().CString());

//    node_->SetWorldPosition(pos);

    player->moveToPos_ = pos;
}

void ClientPrediction::UpdateTurn(float timeStep, uint8_t direction, float speedFactor)
{
    const float speed = GetSpeed(timeStep, BASE_TURN_SPEED, speedFactor);
    if ((direction & AB::GameProtocol::TurnDirectionLeft) == AB::GameProtocol::TurnDirectionLeft)
    {
        Turn(speed);
    }
    if ((direction & AB::GameProtocol::TurnDirectionRight) == AB::GameProtocol::TurnDirectionRight)
    {
        Turn(-speed);
    }
}

void ClientPrediction::Turn(float yAngle)
{
    float deg = RadToDeg(yAngle);
    NormalizeAngle(deg);
    Player* player = node_->GetComponent<Player>();
    float newangle = player->rotateTo_.EulerAngles().y_ + deg;
    TurnAbsolute(newangle);
}

void ClientPrediction::TurnAbsolute(float yAngle)
{
    Player* player = node_->GetComponent<Player>();
    player->rotateTo_.FromEulerAngles(0.0f, yAngle, 0.0f);
//    node_->SetWorldRotation(player->rotateTo_);
}

void ClientPrediction::Update(float timeStep)
{
    LogicComponent::Update(timeStep);

    extern bool gNoClientPrediction;
    if (gNoClientPrediction)
        return;

    Player* player = node_->GetComponent<Player>();

    AB::GameProtocol::CreatureState state = player->GetCreatureState();
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

void ClientPrediction::FixedUpdate(float timeStep)
{
    LogicComponent::FixedUpdate(timeStep);
}

void ClientPrediction::CheckServerPosition(int64_t time, const Vector3& serverPos)
{
    serverTime_ = time;
    serverY_ = serverPos.y_;
    Player* player = node_->GetComponent<Player>();
    const Vector3& currPos = player->moveToPos_;
    float dist = (fabs(currPos.x_ - serverPos.x_) + fabs(currPos.z_ - serverPos.z_)) * 0.5f;
    if (dist > 5.0f)
    {
        double serverTime = player->GetServerTime(time);
        double clientTtime = player->GetClientTime();
        URHO3D_LOGINFOF("Distance %f, server time %f, client time %f", dist, serverTime, clientTtime);
        // Lerp to actual position
        player->moveToPos_ = serverPos;
//        node_->SetWorldPosition(player->moveToPos_);
    }
}

void ClientPrediction::CheckServerRotation(int64_t time, float rad)
{
    serverTime_ = time;
    Player* player = node_->GetComponent<Player>();
    const Quaternion& currRot = player->rotateTo_;
    float deg = RadToDeg(rad);
    NormalizeAngle(deg);
    if (fabs(currRot.EulerAngles().y_ - deg) > 10.0f)
    {
        player->rotateTo_.FromEulerAngles(0.0f, deg, 0.0f);
    }
}
