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

#pragma once

#include "Actor.h"
#include <AB/ProtocolCodes.h>

using namespace Urho3D;

const int CTRL_MOVE_FORWARD = 1;
const int CTRL_MOVE_BACK    = 1 << 1;
const int CTRL_MOVE_LEFT    = 1 << 2;
const int CTRL_MOVE_RIGHT   = 1 << 4;
const int CTRL_TURN_RIGHT   = 1 << 5;
const int CTRL_TURN_LEFT    = 1 << 6;
const int CTRL_MOVE_LOCK    = 1 << 7;

const float CAMERA_MIN_DIST = 0.0f;
const float CAMERA_INITIAL_DIST = 10.0f;
const float CAMERA_MAX_DIST = 40.0f;

/// Character component, responsible for physical movement according to controls, as well as animation.
class Player : public Actor
{
    URHO3D_OBJECT(Player, Actor)
public:
    /// Construct.
    Player(Context* context);
    ~Player() override;

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);
    static Player* CreatePlayer(uint32_t id, Scene* scene,
        const Vector3& position, const Quaternion& rotation,
        AB::GameProtocol::CreatureState state,
        PropReadStream& data);
    void Init(Scene* scene, const Vector3& position, const Quaternion& rotation,
        AB::GameProtocol::CreatureState state) override;

    /// Handle physics world update. Called by LogicComponent base class.
    void FixedUpdate(float timeStep) override;

    /// Movement controls. Assigned by the main program each frame.
    Controls controls_;
    SharedPtr<Node> cameraNode_;
    bool moveLock_{ false };
    float cameraDistance_{ CAMERA_INITIAL_DIST };
    void SetYRotation(int64_t time, float rad, bool updateYaw) override;
    void CameraZoom(bool increase);
    void UpdateYaw();
    void FollowSelected();
    void Attack();
    void GotoPosition(const Vector3& pos);
    void ClickObject(uint32_t objectId);
    void SelectObject(uint32_t objectId);
    void UpdateMumbleContext();
    void UpdateUI();
    uint8_t GetMoveDir();
    uint8_t GetTurnDir();
    void MoveTo(int64_t time, const Vector3& newPos) override;
    float lastYaw_{ 0.0f };
protected:
    void PostUpdate(float timeStep) override;
private:
    SharedPtr<SoundSource3D> footstepsSource_;
    uint8_t lastMoveDir_{ AB::GameProtocol::MoveDirectionNone };
    uint8_t lastTurnDir_{ AB::GameProtocol::TurnDirectionNone };
    void HandleActorNameClicked(StringHash eventType, VariantMap& eventData);
    void HandleSelectSelf(StringHash eventType, VariantMap& eventData);
    void HandleSkillsChanged(StringHash eventType, VariantMap& eventData);
    void SetSkillBarSkills();
};

template <>
inline bool Is<Player>(const GameObject& obj)
{
    return obj.objectType_ == ObjectTypeSelf;
}
