#include "stdafx.h"
#include "GameObject.h"
#include "MathUtils.h"
#include "FwClient.h"
#include "TimeUtils.h"

#include <Urho3D/DebugNew.h>

GameObject::GameObject(Context* context) :
    LogicComponent(context),
    objectType_(ObjectTypeStatic),
    creatureState_(AB::GameProtocol::CreatureStateIdle),
    hovered_(false),
    playerSelected_(false),
    speedFactor_(1.0f)
{
}

GameObject::~GameObject()
{
}

double GameObject::GetClientTime() const
{
    FwClient* c = context_->GetSubsystem<FwClient>();
    return (double)(Client::AbTick() - c->GetClockDiff() - spawnTickServer_) / 1000.0;
}

void GameObject::SetYRotation(float rad, bool)
{
    Quaternion direction;
    float deg = RadToDeg(rad);
    direction.FromAngleAxis(deg, Vector3::UP);
    GetNode()->SetRotation(direction);
}

void GameObject::SetCreatureState(int64_t, AB::GameProtocol::CreatureState newState)
{
    URHO3D_LOGINFOF("New State of object %d; %d", id_, static_cast<int>(newState));
    creatureState_ = newState;
}

void GameObject::SetSpeedFactor(int64_t, float value)
{
    speedFactor_ = value;
}

float GameObject::GetYRotation() const
{
    return DegToRad(GetNode()->GetRotation().YawAngle());
}

void GameObject::MoveTo(int64_t, const Vector3& newPos)
{
    GetNode()->SetPosition(newPos);
}

IntVector2 GameObject::WorldToScreenPoint()
{
    return WorldToScreenPoint(GetNode()->GetPosition());
}

IntVector2 GameObject::WorldToScreenPoint(Vector3 pos)
{
    Node* camNode = GetScene()->GetChild("CameraNode");
    if (!camNode)
        return IntVector2::ZERO;

    Camera* cam = camNode->GetComponent<Camera>();
    if (!cam)
        return IntVector2::ZERO;

    Vector2 screenPoint = cam->WorldToScreenPoint(pos);
    int x;
    int y;
    /// \todo This is incorrect if the viewport is used on a texture rendertarget instead of the backbuffer, as it may have different dimensions.
    Graphics* graphics = GetSubsystem<Graphics>();
    x = (int)(screenPoint.x_ * graphics->GetWidth());
    y = (int)(screenPoint.y_ * graphics->GetHeight());

    return IntVector2(x, y);
}
