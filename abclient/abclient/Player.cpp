#include "stdafx.h"
#include "Player.h"
#include "FwClient.h"
#include "MathUtils.h"

#include <Urho3D/DebugNew.h>

Player::Player(Context* context) :
    Actor(context),
    lastMoveDir_(AB::GameProtocol::MoveDirectionNone),
    lastTurnDir_(AB::GameProtocol::TurnDirectionNone),
    cameraDistance_(CAMERA_INITIAL_DIST)
{
}

void Player::RegisterObject(Context* context)
{
    context->RegisterFactory<Player>();

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Controls Yaw", float, controls_.yaw_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Controls Pitch", float, controls_.pitch_, 0.0f, AM_DEFAULT);
}

Player* Player::CreatePlayer(uint32_t id, Context* context, Scene* scene)
{
    Node* objectNode = scene->CreateChild();
    Player* result = objectNode->CreateComponent<Player>();
    result->id_ = id;

    Node* adjustNode = result->GetNode()->CreateChild("AdjNode");
    adjustNode->SetRotation(Quaternion(180, Vector3(0, 1, 0)));
    result->Init();

    adjustNode->CreateComponent<AnimationController>();
    result->animatedModel_ = adjustNode->CreateComponent<AnimatedModel>();
    result->animatedModel_->SetCastShadows(true);
    adjustNode->CreateComponent<AnimationController>();

    // Create camera
    result->cameraNode_ = scene->CreateChild("CameraNode");
    result->cameraNode_->SetPosition(Vector3(0.0f, 2.0f, -5.0f));
    Camera* camera = result->cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);

    return result;
}

void Player::Init()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    // Load stuff...
//    mesh_ = "/Models/Sphere.mdl";
//    materials_.Push("/Materials/Stone.xml");

    // Create the model
    Actor::Init();

/*
    // Set the head bone for manual control
    if (type_ == Actor::Animated)
    {
        AnimatedModel* animModel = dynamic_cast<AnimatedModel*>(GetModel());
        assert(animModel);
        Bone* headBone = animModel->GetSkeleton().GetBone("Head");
        if (headBone)
            headBone->animated_ = false;
    }

    Node* footstepNode = node_->CreateChild("FootstepsSoundNode");
    footstepsSource_ = footstepNode->CreateComponent<SoundSource3D>();
    footstepsSource_->SetSoundType(SOUND_EFFECT);
    footstepsSource_->SetGain(0.3f);
    */
}

void Player::Update(float timeStep)
{
    Actor::Update(timeStep);

    FwClient* client = context_->GetSubsystem<FwClient>();

    uint8_t moveDir = AB::GameProtocol::MoveDirectionNone;
    if (!(controls_.IsDown(CTRL_MOVE_FORWARD) && controls_.IsDown(CTRL_MOVE_BACK)))
    {
        if (controls_.IsDown(CTRL_MOVE_FORWARD))
            moveDir |= AB::GameProtocol::MoveDirectionNorth;
        if (controls_.IsDown(CTRL_MOVE_BACK))
            moveDir |= AB::GameProtocol::MoveDirectionSouth;
    }
    if (!(controls_.IsDown(CTRL_MOVE_LEFT) && controls_.IsDown(CTRL_MOVE_RIGHT)))
    {
        if (controls_.IsDown(CTRL_MOVE_LEFT))
            moveDir |= AB::GameProtocol::MoveDirectionWest;
        if (controls_.IsDown(CTRL_MOVE_RIGHT))
            moveDir |= AB::GameProtocol::MoveDirectionEast;
    }

    if (lastMoveDir_ != moveDir)
    {
        if (creatureState_ == AB::GameProtocol::CreatureStateIdle && lastYaw_ != controls_.yaw_)
        {
            // Set initial move dir when start moving to camera rotation
            client->SetDirection(DegToRad(controls_.yaw_));
            lastYaw_ = controls_.yaw_;
        }
        client->Move(moveDir);
        lastMoveDir_ = moveDir;
    }

    uint8_t turnDir = AB::GameProtocol::TurnDirectionNone;
    if (!(controls_.IsDown(CTRL_TURN_LEFT) & controls_.IsDown(CTRL_TURN_RIGHT)))
    {
        if (controls_.IsDown(CTRL_TURN_LEFT))
            turnDir |= AB::GameProtocol::TurnDirectionLeft;
        if (controls_.IsDown(CTRL_TURN_RIGHT))
            turnDir |= AB::GameProtocol::TurnDirectionRight;
    }
    if (lastTurnDir_ != turnDir)
    {
        client->Turn(turnDir);
        lastTurnDir_ = turnDir;
    }

    if (creatureState_ == AB::GameProtocol::CreatureStateMoving && fabs(lastYaw_ - controls_.yaw_) > 1.0f)
    {
        // Set initial move dir when start moving to camera rotation
        client->SetDirection(DegToRad(controls_.yaw_));
        lastYaw_ = controls_.yaw_;
    }
}

void Player::SetYRotation(float rad, bool updateYaw)
{
    Actor::SetYRotation(rad, updateYaw);
    // Update camera rotation
    Input* input = GetSubsystem<Input>();

    if (updateYaw)
    {
        // Yaw is already set by the client
        float deg = RadToDeg(GetYRotation());
        controls_.yaw_ = deg;
    }
    lastYaw_ = controls_.yaw_;
}

void Player::SetCameraDist(bool increase)
{
    float diff = cameraDistance_ / 10.0f;
    if (diff < 0.5f)
        diff = 0.5f;
    if (increase)
        cameraDistance_ += diff;
    else
        cameraDistance_ -= diff;
    cameraDistance_ = Clamp(cameraDistance_, CAMERA_MIN_DIST, CAMERA_MAX_DIST);
}

void Player::PostUpdate(float timeStep)
{
    Node* characterNode = GetNode();

    // Get camera lookat dir from character yaw + pitch
    Quaternion rot = Quaternion(controls_.yaw_, Vector3::UP);
    Quaternion dir = rot * Quaternion(controls_.pitch_, Vector3::RIGHT);

    // Third person camera: position behind the character
    static const Vector3 CAM_POS(0.0f, 2.0f, 0.0f);
    Vector3 aimPoint = characterNode->GetPosition() + rot * CAM_POS;

    Vector3 rayDir = dir * Vector3::BACK;
    float rayDistance = cameraDistance_;
    rayDistance = Clamp(rayDistance, CAMERA_MIN_DIST, CAMERA_MAX_DIST);

    cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);
    cameraNode_->SetRotation(dir);
}

