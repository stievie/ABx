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

#include "GameObject.h"
#include "MathUtils.h"
#include "FwClient.h"
#include "TimeUtils.h"
#include "LevelManager.h"
#include "Player.h"
#include <sa/time.h>

//#include <Urho3D/DebugNew.h>

GameObject::GameObject(Context* context) :
    LogicComponent(context)
{
}

GameObject::~GameObject()
{
}

double GameObject::GetClientTime() const
{
    FwClient* c = GetSubsystem<FwClient>();
    return static_cast<double>(sa::time::tick() - c->GetClockDiff() - spawnTickServer_) / 1000.0;
}

void GameObject::SetYRotation(int64_t, float rad, bool)
{
    const float deg = NormalizedAngle(RadToDeg(rad));
    GetNode()->SetRotation({ deg, Vector3::UP });
}

void GameObject::SetCreatureState(int64_t, AB::GameProtocol::CreatureState newState)
{
//    URHO3D_LOGINFOF("New State of object %d; %d", gameId_, static_cast<int>(newState));
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

void GameObject::ForcePosition(int64_t, const Vector3& newPos)
{
    GetNode()->SetWorldPosition(newPos);
}

float GameObject::GetDistance(const Vector3& pos) const
{
    return GetNode()->GetWorldPosition().DistanceToPoint(pos);
}

float GameObject::GetDistanceToPlayer() const
{
    if (Is<Player>(this))
        return 0.0f;
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return std::numeric_limits<float>::max();
    return GetDistance(player->GetNode()->GetWorldPosition());
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

    const Vector2 screenPoint = cam->WorldToScreenPoint(pos);
    int x;
    int y;
    /// \todo This is incorrect if the viewport is used on a texture rendertarget instead of the backbuffer, as it may have different dimensions.
    Graphics* graphics = GetSubsystem<Graphics>();
    x = static_cast<int>(screenPoint.x_ * graphics->GetWidth());
    y = static_cast<int>(screenPoint.y_ * graphics->GetHeight());

    return IntVector2(x, y);
}
