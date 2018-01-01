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
const float CAMERA_INITIAL_DIST = 10.0f;
const float CAMERA_MAX_DIST = 40.0f;

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
    void SetYRotation(float rad, bool updateYaw) override;
    void SetCameraDist(bool increase);
protected:
    void PostUpdate(float timeStep) override;
private:
    SharedPtr<SoundSource3D> footstepsSource_;
    uint8_t lastMoveDir_;
    uint8_t lastTurnDir_;
    float lastYaw_;
};
