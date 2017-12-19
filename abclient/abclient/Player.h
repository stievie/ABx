//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

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

const float CAMERA_MIN_DIST = 0.0f;
const float CAMERA_INITIAL_DIST = 5.0f;
const float CAMERA_MAX_DIST = 30.0f;

/// Character component, responsible for physical movement according to controls, as well as animation.
class Player : public Actor
{
    URHO3D_OBJECT(Player, Actor);

public:
    /// Construct.
    Player(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);
    static Player* CreatePlayer(uint32_t id, Context* context, Scene* scene);
    void Init() override;

    /// Handle physics world update. Called by LogicComponent base class.
    void Update(float timeStep) override;

    /// Movement controls. Assigned by the main program each frame.
    Controls controls_;
    SharedPtr<Node> cameraNode_;
    float cameraDistance_;
    void SetYRotation(float rad) override;
    void SetCameraDist(bool increase);
protected:
    void PostUpdate(float timeStep) override;
private:
    SharedPtr<SoundSource3D> footstepsSource_;
    uint8_t lastMoveDir_;
    uint8_t lastTurnDir_;
    float lastYaw_;
};
