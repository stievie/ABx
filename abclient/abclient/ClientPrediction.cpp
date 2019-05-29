#include "stdafx.h"
#include "ClientPrediction.h"
#include "Player.h"

void ClientPrediction::RegisterObject(Context* context)
{
    context->RegisterFactory<ClientPrediction>();
}

ClientPrediction::ClientPrediction(Context* context) :
    LogicComponent(context),
    serverTime_(0),
    serverY_(0.0f)
{
    SetUpdateEventMask(USE_UPDATE);
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
    Quaternion oriention = node_->GetWorldRotation();
    Vector3 pos = node_->GetWorldPosition();
    const Matrix3 m = oriention.RotationMatrix();
    const Vector3 a = amount * speed;
    const Vector3 v = m * a;
    pos += v;
    if (!Equals(serverY_, 0.0f))
        pos.y_ = serverY_;
//    URHO3D_LOGINFOF("speed %f, amount %s, v %s, new pos %s", speed, amount.ToString().CString(), v.ToString().CString(), pos.ToString().CString());

    Player* player = node_->GetComponent<Player>();
    node_->SetWorldPosition(pos);
    player->moveToPos_ = pos;
}

void ClientPrediction::Update(float timeStep)
{
    LogicComponent::Update(timeStep);

    Player* player = node_->GetComponent<Player>();
    AB::GameProtocol::CreatureState state = player->GetCreatureState();
    if (state != AB::GameProtocol::CreatureStateIdle && state != AB::GameProtocol::CreatureStateMoving)
        return;
    uint8_t moveDir = player->GetMoveDir();
    if (moveDir != 0 && state == AB::GameProtocol::CreatureStateIdle)
        player->SetCreatureState(serverTime_, AB::GameProtocol::CreatureStateMoving);

    UpdateMove(timeStep, moveDir, player->GetSpeedFactor());
//    uint8_t turnDir = player->GetTurnDir();
}

void ClientPrediction::CheckServerPosition(int64_t time, const Vector3& serverPos)
{
    serverTime_ = time;
    serverY_ = serverPos.y_;
    Player* player = node_->GetComponent<Player>();
    const Vector3& currPos = node_->GetWorldPosition();
    float dist = currPos.DistanceToPoint(serverPos);
    if (dist > 5.0f)
    {
        URHO3D_LOGINFOF("Distance %f", dist);
        player->moveToPos_ = serverPos;
    }
}
