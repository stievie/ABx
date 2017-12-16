#include "stdafx.h"
#include "Player.h"
#include "FwClient.h"

Player::Player(Context* context) :
    Actor(context),
    lastDir_(AB::GameProtocol::MoveDirectionNone)
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
    Node* objectNode = scene->CreateChild(id, Urho3D::LOCAL, false);
    Player* result = objectNode->CreateComponent<Player>();

    Node* adjustNode = result->GetNode()->CreateChild("AdjNode");
    adjustNode->SetRotation(Quaternion(180, Vector3(0, 1, 0)));
    result->Init();

    adjustNode->CreateComponent<AnimationController>();
    result->animatedModel_ = adjustNode->CreateComponent<AnimatedModel>();
    result->animatedModel_->SetCastShadows(true);
    adjustNode->CreateComponent<AnimationController>();

    // Create camera
    result->cameraNode_ = result->GetNode()->CreateChild("CameraNode");
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

    uint8_t dir = AB::GameProtocol::MoveDirectionNone;
    if (controls_.IsDown(CTRL_FORWARD))
        dir |= AB::GameProtocol::MoveDirectionNorth;
    if (controls_.IsDown(CTRL_BACK))
        dir |= AB::GameProtocol::MoveDirectionSouth;
    if (controls_.IsDown(CTRL_LEFT))
        dir |= AB::GameProtocol::MoveDirectionWest;
    if (controls_.IsDown(CTRL_RIGHT))
        dir |= AB::GameProtocol::MoveDirectionEast;

    if (lastDir_ != dir)
    {
        client->Move(dir);
        lastDir_ = dir;
    }
}

