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

#include "stdafx.h"

#include "Player.h"

Player::Player(Context* context) :
    Actor(context)
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
    Player* result = new Player(context);
    result->objectNode_ = scene->CreateChild("Player", Urho3D::REPLICATED, id);

    Node* adjustNode = result->objectNode_->CreateChild("AdjNode");
    adjustNode->SetRotation(Quaternion(180, Vector3(0, 1, 0)));
    result->Init();

    adjustNode->CreateComponent<AnimationController>();
    result->animatedModel_ = adjustNode->CreateComponent<AnimatedModel>();
    result->animatedModel_->SetCastShadows(true);
    adjustNode->CreateComponent<AnimationController>();

    // Create camera
    result->cameraNode_ = result->objectNode_->CreateChild("CameraNode");
    result->cameraNode_->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
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

    // Update movement & animation
    const Quaternion& rot = node_->GetRotation();
    Vector3 moveDir = Vector3::ZERO;

    if (controls_.IsDown(CTRL_FORWARD))
        moveDir -= Vector3::FORWARD;
    if (controls_.IsDown(CTRL_BACK))
        moveDir -= Vector3::BACK * 2.0f;
    if (controls_.IsDown(CTRL_LEFT))
        moveDir -= Vector3::LEFT;
    if (controls_.IsDown(CTRL_RIGHT))
        moveDir -= Vector3::RIGHT;

    // Normalize move vector so that diagonal strafing is not faster
    if (moveDir.LengthSquared() > 0.0f)
        moveDir.Normalize();
}

