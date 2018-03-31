#include "stdafx.h"
#include "GameObject.h"
#include "MathUtils.h"

#include <Urho3D/DebugNew.h>

GameObject::GameObject(Context* context) :
    LogicComponent(context),
    objectType_(ObjectTypeStatic),
    creatureState_(AB::GameProtocol::CreatureStateIdle),
    hovered_(false),
    playerSelected_(false)
{
}

GameObject::~GameObject()
{
}

void GameObject::SetYRotation(float rad, bool updateYaw)
{
    Quaternion direction;
    float deg = RadToDeg(rad);
    direction.FromAngleAxis(deg, Vector3::UP);
    GetNode()->SetRotation(direction);
}

void GameObject::SetCreatureState(double time, AB::GameProtocol::CreatureState newState)
{
    creatureState_ = newState;
}

float GameObject::GetYRotation()
{
    return DegToRad(GetNode()->GetRotation().YawAngle());
}

void GameObject::MoveTo(double time, const Vector3 & newPos)
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
