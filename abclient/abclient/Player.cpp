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


#include "Player.h"
#include "ActorResourceBar.h"
#include "ClientPrediction.h"
#include "EquipmentWindow.h"
#include "FwClient.h"
#include "LevelManager.h"
#include "MathUtils.h"
#include "Mumble.h"
#include "Shortcuts.h"
#include "SkillBarWindow.h"
#include "TimeUtils.h"
#include "WindowManager.h"
#include "WorldLevel.h"
#include <Urho3D/Container/Sort.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <abshared/Attributes.h>
#include <abshared/Mechanic.h>
#include <algorithm>

//#include <Urho3D/DebugNew.h>

Player::Player(Context* context) :
    Actor(context)
{
    SetUpdateEventMask(USE_FIXEDUPDATE | USE_POSTUPDATE | USE_UPDATE);
    SubscribeToEvent(Events::E_ACTORNAMECLICKED, URHO3D_HANDLER(Player, HandleActorNameClicked));
    SubscribeToEvent(Events::E_ACTORNAMEDOUBLECLICKED, URHO3D_HANDLER(Player, HandleActorNameDoubleClicked));
    SubscribeToEvent(Events::E_SC_SELECTSELF, URHO3D_HANDLER(Player, HandleSelectSelf));
    SubscribeToEvent(Events::E_ACTOR_SKILLS_CHANGED, URHO3D_HANDLER(Player, HandleSkillsChanged));
    SubscribeToEvent(Events::E_SC_SELECTCLOSESTFOE, URHO3D_HANDLER(Player, HandleSelectClosestFoe));
    SubscribeToEvent(Events::E_SC_SELECTCLOSESTFRIEND, URHO3D_HANDLER(Player, HandleSelectClosestAlly));
    SubscribeToEvent(Events::E_SC_SELECTNEXTFOE, URHO3D_HANDLER(Player, HandleSelectNextFoe));
    SubscribeToEvent(Events::E_SC_SELECTPREVFOE, URHO3D_HANDLER(Player, HandleSelectPrevFoe));
    SubscribeToEvent(Events::E_SC_SELECTNEXTALLY, URHO3D_HANDLER(Player, HandleSelectNextAlly));
    SubscribeToEvent(Events::E_SC_SELECTPREVALLY, URHO3D_HANDLER(Player, HandleSelectPrevAlly));
}

Player::~Player()
{ }

void Player::RegisterObject(Context* context)
{
    context->RegisterFactory<Player>();

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Controls Yaw", float, controls_.yaw_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Controls Pitch", float, controls_.pitch_, 0.0f, AM_DEFAULT);
}

Player* Player::CreatePlayer(uint32_t id, Scene* scene)
{
    Node* node = scene->CreateChild(0, LOCAL);
    Player* result = node->CreateComponent<Player>();
    node->CreateComponent<ClientPrediction>();
    result->gameId_ = id;
    return result;
}

void Player::Init(Scene* scene, const Vector3& position, const Quaternion& rotation,
    AB::GameProtocol::CreatureState state)
{
    Actor::Init(scene, position, rotation, state);
    RigidBody* body = node_->GetComponent<RigidBody>(true);
    body->SetCollisionLayer(1);
    AnimatedModel* animModel = node_->GetComponent<AnimatedModel>(true);
    if (animModel)
    {
        Bone* headBone = animModel->GetSkeleton().GetBone("Head");
        if (headBone)
        {
            headBone->animated_ = false;
            Node* headNode = node_->GetChild("Head", true);
            if (headNode)
            {
                auto* faceLightNode = headNode->CreateChild("FaceLightNode", LOCAL);
                faceLightNode->SetPosition({ 0.0f, 0.0f, -0.5f });
                faceLight_ = faceLightNode->CreateComponent<Light>();
                faceLight_->SetColor({ 0.96f, 0.8f, 0.64f });
                faceLight_->SetLightType(LIGHT_SPOT);
                faceLight_->SetBrightness(0.45f);
                faceLight_->SetRange(3.0f);
                faceLight_->SetFov(60.0f);
            }
        }
    }
    // Create camera
    Options* options = GetSubsystem<Options>();
    cameraNode_ = scene->CreateChild("CameraNode");
    cameraNode_->SetPosition(Vector3(0.0f, 2.0f, -5.0f));
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(options->GetCameraFarClip());
    camera->SetNearClip(options->GetCameraNearClip());
    camera->SetFov(options->GetCameraFov());

    // Update Mumble
    Mumble* mumble = GetSubsystem<Mumble>();
    if (mumble)
    {
        mumble->SetAvatar(SharedPtr<Node>(GetNode()));
        mumble->SetCamera(cameraNode_);
        mumble->SetIdentity(name_);
    }

    // Set skills
    SetSkillBarSkills();
}

void Player::CreateSoundListener()
{
    if (soundListenerNode_)
        soundListenerNode_->Remove();

    // Add sound listener to camera node, also Guild Wars does it so.
    auto* options = GetSubsystem<Options>();
    Node* parentNode = nullptr;
    if (options->soundListenerToHead_)
    {
        parentNode = GetNode()->GetChild("Head", true);
        if (!parentNode)
            parentNode = GetNode();
    }
    else
        parentNode = cameraNode_;

    soundListenerNode_ = parentNode->CreateChild("SoundListenerNode");
    // Let's face the sound
    soundListenerNode_->SetDirection(Vector3(0.0f, M_HALF_PI, 0.0f));
    SoundListener* soundListener = soundListenerNode_->CreateComponent<SoundListener>();
    auto* audio = GetSubsystem<Audio>();
    audio->SetListener(soundListener);
}

void Player::HandleSkillsChanged(StringHash, VariantMap& eventData)
{
    using namespace Events::ActorSkillsChanged;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    if (objectId != gameId_)
        return;
    SetSkillBarSkills();
}

void Player::GetFoeSelectionCandidates()
{
    foeSelectionCandidates_.clear();
    foeSelectedIndex_ = -1;
    Octree* world = GetScene()->GetComponent<Octree>();
    if (!world)
        return;

    auto* lm = GetSubsystem<LevelManager>();
    auto* level = lm->GetCurrentLevel<WorldLevel>();
    if (!level)
        return;

    PODVector<Drawable*> result;
    SphereOctreeQuery query(result, { node_->GetPosition(), Game::RANGE_COMPASS });
    world->GetDrawables(query);
    const Vector3 pos = GetNode()->GetPosition();
    for (auto* drawable : result)
    {
        auto* object = level->GetObjectFromNode(drawable->GetNode());
        if (!object || object == this)
            continue;

        if (!Is<Actor>(object))
            continue;

        const auto* actor = To<Actor>(object);
        if (actor->gameId_ != 0 && IsEnemy(actor) && !actor->IsDead())
        {
            float dist = pos.DistanceToPoint(object->GetNode()->GetPosition());
            foeSelectionCandidates_.push_back({ dist, object->gameId_ });
        }
    }
    std::sort(foeSelectionCandidates_.begin(), foeSelectionCandidates_.end(), [](const DistanceId& a, const DistanceId& b)
    {
        return a.distance < b.distance;
    });
}

void Player::GetFriendSelectionCandidates()
{
    friendSelectionCandidates_.clear();
    friendSelectedIndex_ = -1;
    Octree* world = GetScene()->GetComponent<Octree>();
    if (!world)
        return;

    auto* lm = GetSubsystem<LevelManager>();
    auto* level = lm->GetCurrentLevel<WorldLevel>();
    if (!level)
        return;

    PODVector<Drawable*> result;
    SphereOctreeQuery query(result, { node_->GetPosition(), Game::RANGE_COMPASS });
    world->GetDrawables(query);
    const Vector3 pos = GetNode()->GetPosition();
    for (auto* drawable : result)
    {
        auto* object = level->GetObjectFromNode(drawable->GetNode());
        if (!object || object == this)
            continue;

        if (!Is<Actor>(object))
            continue;

        const auto* actor = To<Actor>(object);
        if (actor->gameId_ != 0 && IsAlly(actor) && !actor->IsDead())
        {
            float dist = pos.DistanceToPoint(object->GetNode()->GetPosition());
            friendSelectionCandidates_.push_back({ dist, object->gameId_ });
        }
    }
    std::sort(friendSelectionCandidates_.begin(), friendSelectionCandidates_.end(), [](const DistanceId& a, const DistanceId& b)
    {
        return a.distance < b.distance;
    });
}

void Player::HandleSelectClosestAlly(StringHash, VariantMap&)
{
    friendSelectionCandidates_.clear();
    GetFriendSelectionCandidates();
    lastFriendSelect_ = Client::AbTick();
    if (friendSelectionCandidates_.size() == 0)
        return;
    friendSelectedIndex_ = 0;
    uint32_t id = friendSelectionCandidates_.at(static_cast<unsigned>(friendSelectedIndex_)).id;
    SelectObject(id);
}

void Player::HandleSelectClosestFoe(StringHash, VariantMap&)
{
    foeSelectionCandidates_.clear();
    GetFoeSelectionCandidates();
    lastFoeSelect_ = Client::AbTick();
    if (foeSelectionCandidates_.size() == 0)
        return;
    foeSelectedIndex_ = 0;
    uint32_t id = foeSelectionCandidates_.at(static_cast<unsigned>(foeSelectedIndex_)).id;
    SelectObject(id);
}

void Player::HandleSelectNextFoe(StringHash, VariantMap&)
{
    if (lastFoeSelect_ == 0 || (Client::AbTick() - lastFoeSelect_ > 2000) || foeSelectionCandidates_.empty() || foeSelectedIndex_ == -1)
        GetFoeSelectionCandidates();
    if (foeSelectionCandidates_.size() == 0)
        return;

    lastFoeSelect_ = Client::AbTick();
    ++foeSelectedIndex_;
    if (static_cast<unsigned>(foeSelectedIndex_) > foeSelectionCandidates_.size() - 1)
        foeSelectedIndex_ = 0;

    uint32_t id = foeSelectionCandidates_.at(static_cast<unsigned>(foeSelectedIndex_)).id;
    SelectObject(id);
}

void Player::HandleSelectPrevFoe(StringHash, VariantMap&)
{
    if (lastFoeSelect_ == 0 || (Client::AbTick() - lastFoeSelect_ > 2000) || foeSelectionCandidates_.empty() || foeSelectedIndex_ == -1)
        GetFoeSelectionCandidates();
    if (foeSelectionCandidates_.size() == 0)
        return;

    lastFoeSelect_ = Client::AbTick();

    --foeSelectedIndex_;
    if (foeSelectedIndex_ < 0)
        foeSelectedIndex_ = static_cast<int>(foeSelectionCandidates_.size()) - 1;

    uint32_t id = foeSelectionCandidates_.at(static_cast<size_t>(foeSelectedIndex_)).id;
    SelectObject(id);
}

void Player::HandleSelectNextAlly(StringHash, VariantMap&)
{
    if (lastFriendSelect_ == 0 || (Client::AbTick() - lastFriendSelect_ > 2000) || friendSelectionCandidates_.empty() || friendSelectedIndex_ == -1)
        GetFriendSelectionCandidates();
    if (friendSelectionCandidates_.size() == 0)
        return;

    lastFriendSelect_ = Client::AbTick();
    ++friendSelectedIndex_;
    if (static_cast<unsigned>(friendSelectedIndex_) > friendSelectionCandidates_.size() - 1)
        friendSelectedIndex_ = 0;

    uint32_t id = foeSelectionCandidates_.at(static_cast<unsigned>(friendSelectedIndex_)).id;
    SelectObject(id);
}

void Player::HandleSelectPrevAlly(StringHash, VariantMap&)
{
    if (lastFriendSelect_ == 0 || (Client::AbTick() - lastFriendSelect_ > 2000) || friendSelectionCandidates_.empty() || friendSelectedIndex_ == -1)
        GetFriendSelectionCandidates();
    if (friendSelectionCandidates_.size() == 0)
        return;

    lastFriendSelect_ = Client::AbTick();
    --friendSelectedIndex_;
    if (friendSelectedIndex_ < 0)
        friendSelectedIndex_ = static_cast<int>(friendSelectionCandidates_.size()) - 1;

    uint32_t id = foeSelectionCandidates_.at(static_cast<size_t>(friendSelectedIndex_)).id;
    SelectObject(id);
}

void Player::SetSkillBarSkills()
{
    WindowManager* winMan = GetSubsystem<WindowManager>();
    SkillBarWindow* skillsWin = dynamic_cast<SkillBarWindow*>(winMan->GetWindow(WINDOW_SKILLBAR).Get());
    skillsWin->SetSkills(skills_);
}

uint8_t Player::GetMoveDir()
{
    uint8_t moveDir = AB::GameProtocol::MoveDirectionNone;
    if (!(controls_.IsDown(CTRL_MOVE_FORWARD) && controls_.IsDown(CTRL_MOVE_BACK)))
    {
        if (controls_.IsDown(CTRL_MOVE_FORWARD) || controls_.IsDown(CTRL_MOVE_LOCK))
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
    return moveDir;
}

uint8_t Player::GetTurnDir()
{
    uint8_t turnDir = AB::GameProtocol::TurnDirectionNone;
    if (!(controls_.IsDown(CTRL_TURN_LEFT) && controls_.IsDown(CTRL_TURN_RIGHT)))
    {
        if (controls_.IsDown(CTRL_TURN_LEFT))
            turnDir |= AB::GameProtocol::TurnDirectionLeft;
        if (controls_.IsDown(CTRL_TURN_RIGHT))
            turnDir |= AB::GameProtocol::TurnDirectionRight;
    }
    return turnDir;
}

void Player::MoveTo(int64_t time, const Vector3& newPos)
{
    // Server position
    extern bool gNoClientPrediction;

    if (gNoClientPrediction || autoRun_)
    {
        Actor::MoveTo(time, newPos);
        return;
    }

    ClientPrediction* cp = GetComponent<ClientPrediction>();
    cp->CheckServerPosition(time, newPos);
}

void Player::FixedUpdate(float timeStep)
{
    FwClient* client = GetSubsystem<FwClient>();

    uint8_t moveDir = GetMoveDir();

    if (lastMoveDir_ != moveDir)
    {
        if (creatureState_ == AB::GameProtocol::CreatureState::Idle && !Equals(lastYaw_, controls_.yaw_))
        {
            // Set initial move dir when start moving to camera rotation
            client->SetDirection(DegToRad(controls_.yaw_));
            lastYaw_ = controls_.yaw_;
        }
        client->Move(moveDir);
        lastMoveDir_ = moveDir;
    }

    uint8_t turnDir = GetTurnDir();
    if (lastTurnDir_ != turnDir)
    {
        client->Turn(turnDir);
        lastTurnDir_ = turnDir;
    }

    if (creatureState_ == AB::GameProtocol::CreatureState::Moving && fabs(lastYaw_ - controls_.yaw_) > 1.0f)
    {
        // Set initial move dir when start moving to camera rotation
        client->SetDirection(DegToRad(controls_.yaw_));
        lastYaw_ = controls_.yaw_;
    }

    Actor::FixedUpdate(timeStep);

    // Also Update here. The client takes care that it doesn't send too often.
    client->Update(timeStep);
}

void Player::SetYRotation(int64_t time, float rad, bool updateYaw)
{
    if (updateYaw)
    {
        // Yaw is already set by the client
        float deg = RadToDeg(rad);
        controls_.yaw_ = deg;
        NormalizeAngle(controls_.yaw_);
    }
    lastYaw_ = controls_.yaw_;

    extern bool gNoClientPrediction;

    if (gNoClientPrediction || autoRun_)
    {
        Actor::SetYRotation(time, rad, updateYaw);
        return;
    }

    if (GetTurnDir() == 0)
        Actor::SetYRotation(time, rad, updateYaw);

    ClientPrediction* cp = GetComponent<ClientPrediction>();
    cp->CheckServerRotation(time, rad);
}

void Player::CameraZoom(bool increase)
{
    float diff = Max(cameraDistance_ / 10.0f, 0.2f);
    if (increase)
        cameraDistance_ += diff;
    else
        cameraDistance_ -= diff;
    cameraDistance_ = Clamp(cameraDistance_, 0.0f, CAMERA_MAX_DIST);
}

void Player::UpdateYaw()
{
    const Quaternion& rot = node_->GetRotation();
    controls_.yaw_ = rot.EulerAngles().y_;
    lastYaw_ = controls_.yaw_;
}

void Player::Interact()
{
    GetSubsystem<FwClient>()->Interact();
}

void Player::GotoPosition(const Vector3& pos)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->GotoPos(pos);
}

void Player::ClickObject(uint32_t objectId)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->ClickObject(gameId_, objectId);
}

void Player::SelectObject(uint32_t objectId)
{
    if (objectId == GetSelectedObjectId())
        return;
    FwClient* client = GetSubsystem<FwClient>();
    client->SelectObject(gameId_, objectId);
}

void Player::PostUpdate(float timeStep)
{
    Node* characterNode = GetNode();
    Shortcuts* scs = GetSubsystem<Shortcuts>();

    float yaw = controls_.yaw_;
    if (scs->IsTriggered(Events::E_SC_REVERSECAMERA))
        yaw += 180.0f;
    // Get camera look at dir from character yaw + pitch
    const Quaternion rot = Quaternion(yaw, Vector3::UP);
    Quaternion dir = rot * Quaternion(controls_.pitch_, Vector3::RIGHT);

    Node* headNode = characterNode->GetChild("Head", true);
    Vector3 headWorldPos;

    const bool firstPerson = cameraDistance_ < CAMERA_MIN_DIST;

    // Player looks in camera direstion or into the camera
    if (headNode)
    {
        headWorldPos = headNode->GetWorldPosition();

        if (!firstPerson)
        {
            static constexpr float YAW_LIMIT = 60.0f;
            static constexpr float PITCH_LIMIT = 30.0f;
            // Turn head to camera pitch, but limit to avoid unnatural animation
            float limitPitch = Clamp(controls_.pitch_, -PITCH_LIMIT, PITCH_LIMIT);
            float yaw2 = controls_.yaw_ - characterNode->GetRotation().YawAngle();
            // When the camera is in front of the player, i.e. looking into the face of the player, make the player look forward.
            bool lookToCam = fabs(yaw2 - 180.0f) < YAW_LIMIT;
            if (lookToCam)
            {
                yaw2 -= 180.0f;
                limitPitch = -limitPitch;
            }

            float yaw3 = yaw2 - floor((yaw2 + 180.0f) / 360.0f) * 360.0f;
            float limitYaw = Clamp(yaw3, -YAW_LIMIT, YAW_LIMIT);
            const Quaternion headDir = characterNode->GetRotation() *
                Quaternion(limitYaw, Vector3::UP) *
                Quaternion(limitPitch, Vector3::RIGHT);
            // This could be expanded to look at an arbitrary target, now just look at a point in front
            const Vector3 headWorldTarget = headWorldPos + headDir * Vector3::BACK;
            headNode->LookAt(headWorldTarget);
        }
    }

    // Third person camera: position behind the character
    Vector3 aimPoint;
    if (GetSubsystem<Options>()->stickCameraToHead_ && headNode)
        aimPoint = headWorldPos;
    else
    {
        static const Vector3 CAM_POS(0.0f, 1.5f, 0.0f);
        aimPoint = characterNode->GetWorldPosition() + rot * CAM_POS;
    }

    float rayDistance = cameraDistance_;

    if (firstPerson)
    {
        // Hide Player model in 1st person perspective
        model_->SetViewMask(0);
        // Snap to 1st person perspective under some threshold
        rayDistance = 0.0f;
    }
    else if (model_->GetViewMask() == 0)
        model_->SetViewMask(static_cast<unsigned>(-1));

    Vector3 newCamPos = aimPoint;

    // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
    // Check the camera is not too close to the player model.
    if (!firstPerson)
    {
        // Not in 1st person prespective, means the camera can collide with other objects.
        const Vector3 rayDir = dir * Vector3::BACK;
        PhysicsRaycastResult result;
        node_->GetScene()->GetComponent<PhysicsWorld>()->RaycastSingle(result,
            Ray(aimPoint, rayDir), rayDistance + 0.5f, COLLISION_LAYER_CAMERA);
        rayDistance = Clamp(Min(result.distance_ - 1.5f, rayDistance), CAMERA_MIN_DIST, CAMERA_MAX_DIST);

        newCamPos = aimPoint + rayDir * rayDistance;
    }

    if ((oldCamPos_ != newCamPos || oldCamPos_ == Vector3::ZERO) ||
        (oldCamDir_ != dir || oldCamDir_ == Quaternion::IDENTITY))
    {
        if (oldCamPos_ != Vector3::ZERO)
            newCamPos = oldCamPos_.Lerp(newCamPos, 0.3f);
        cameraNode_->SetPosition(newCamPos);
        oldCamPos_ = newCamPos;
        if (oldCamDir_ != Quaternion::IDENTITY)
            dir = oldCamDir_.Slerp(dir, 0.3f);
        cameraNode_->SetRotation(dir);
        oldCamDir_ = dir;
    }

    Actor::PostUpdate(timeStep);
}

void Player::HandleActorNameClicked(StringHash, VariantMap& eventData)
{
    using namespace Events::ActorNameClicked;
    uint32_t id = eventData[P_SOURCEID].GetUInt();
    if (id != gameId_)
    {
        ClickObject(id);
        SelectObject(id);
    }
}

void Player::HandleActorNameDoubleClicked(StringHash, VariantMap& eventData)
{
    using namespace Events::ActorNameDoubleClicked;
    uint32_t id = eventData[P_SOURCEID].GetUInt();
    if (id != gameId_)
    {
        Interact();
    }
}

void Player::HandleSelectSelf(StringHash, VariantMap&)
{
    SelectObject(gameId_);
}

void Player::UpdateUI()
{
    WindowManager* wm = GetSubsystem<WindowManager>();
    if (auto* w = wm->GetWindow<EquipmentWindow>(WINDOW_EQUIPMENT))
        w->UpdateEquipment(this);
    if (auto* w = wm->GetWindow<ActorHealthBar>(WINDOW_HEALTHBAR))
        w->SetActor(SharedPtr<Actor>(this));
    if (auto* w = wm->GetWindow<ActorEnergyBar>(WINDOW_ENERGYBAR))
        w->SetActor(SharedPtr<Actor>(this));
}

void Player::UpdateMumbleContext()
{
    auto* mumble = GetSubsystem<Mumble>();
    if (!mumble)
        return;

    auto* lm = GetSubsystem<LevelManager>();
    String ctx(lm->GetInstanceUuid());
    if (ctx.Length() == 0)
        return;
    ctx.AppendWithFormat(":%d", groupId_);
    mumble->SetContext(ctx);
}
