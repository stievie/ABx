#include "stdafx.h"
#include "Player.h"
#include "FwClient.h"
#include "MathUtils.h"
#include "Options.h"
#include "AbEvents.h"
#include "Shortcuts.h"
#include "WindowManager.h"
#include "SkillBarWindow.h"
#include "Mumble.h"

#include <Urho3D/DebugNew.h>

Player::Player(Context* context) :
    Actor(context),
    lastMoveDir_(AB::GameProtocol::MoveDirectionNone),
    lastTurnDir_(AB::GameProtocol::TurnDirectionNone),
    cameraDistance_(CAMERA_INITIAL_DIST),
    moveLock_(false),
    stickCameraToHead_(false)
{
    SetUpdateEventMask(USE_FIXEDUPDATE | USE_POSTUPDATE | USE_UPDATE);
    Options* opt = GetSubsystem<Options>();
    stickCameraToHead_ = opt->stickCameraToHead_;
    SubscribeToEvent(AbEvents::E_ACTORNAMECLICKED, URHO3D_HANDLER(Player, HandleActorNameClicked));
    SubscribeToEvent(AbEvents::E_SC_SELECTSELF, URHO3D_HANDLER(Player, HandleSelectSelf));
    FwClient* cli = GetSubsystem<FwClient>();
    cli->UnsubscribeUpdate();
}

Player::~Player()
{
    FwClient* cli = GetSubsystem<FwClient>();
    if (cli)
        cli->SubscribeUpdate();
}

void Player::RegisterObject(Context* context)
{
    context->RegisterFactory<Player>();

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Controls Yaw", float, controls_.yaw_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Controls Pitch", float, controls_.pitch_, 0.0f, AM_DEFAULT);
}

Player* Player::CreatePlayer(uint32_t id, Scene* scene,
    const Vector3& position, const Quaternion& rotation,
    AB::GameProtocol::CreatureState state,
    PropReadStream& data)
{
    Node* node = scene->CreateChild(0, LOCAL);
    Player* result = node->CreateComponent<Player>();
    result->id_ = id;

    result->Unserialize(data);
    result->Init(scene, position, rotation, state);

    result->PlayAnimation(ANIM_IDLE, true, 0.0f);
    return result;
}

void Player::Init(Scene* scene, const Vector3& position, const Quaternion& rotation,
    AB::GameProtocol::CreatureState state)
{
    Actor::Init(scene, position, rotation, state);
    RigidBody* body = node_->GetComponent<RigidBody>(true);
    body->SetCollisionLayer(1);
#ifdef PLAYER_HEAD_ANIMATION
    AnimatedModel* animModel = node_->GetComponent<AnimatedModel>(true);
    if (animModel)
    {
        Bone* headBone = animModel->GetSkeleton().GetBone("Head");
        if (headBone)
            headBone->animated_ = false;
    }
#endif
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
        SharedPtr<Node> charNode = SharedPtr<Node>(GetNode());
        mumble->SetAvatar(charNode);
        mumble->SetCamera(cameraNode_);
        mumble->SetIdentity(name_);
    }

    // Set skills
    WindowManager* winMan = GetSubsystem<WindowManager>();
    SkillBarWindow* skillsWin = dynamic_cast<SkillBarWindow*>(winMan->GetWindow(WINDOW_SKILLBAR).Get());
    skillsWin->SetSkills(skills_);
}

void Player::FixedUpdate(float timeStep)
{
    FwClient* client = context_->GetSubsystem<FwClient>();

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
    }
    lastYaw_ = controls_.yaw_;

    Actor::SetYRotation(time, rad, updateYaw);
}

void Player::CameraZoom(bool increase)
{
    float diff = Max(cameraDistance_ / 10.0f, 0.2f);
    if (increase)
        cameraDistance_ += diff;
    else
        cameraDistance_ -= diff;
    cameraDistance_ = Clamp(cameraDistance_, CAMERA_MIN_DIST, CAMERA_MAX_DIST);
}

void Player::UpdateYaw()
{
    const Quaternion& rot = node_->GetRotation();
    controls_.yaw_ = rot.YawAngle();
    lastYaw_ = controls_.yaw_;
}

void Player::FollowSelected()
{
    if (auto so = selectedObject_.Lock())
    {
        FwClient* client = context_->GetSubsystem<FwClient>();
        client->FollowObject(so->id_);
    }
}

void Player::GotoPosition(const Vector3& pos)
{
    FwClient* client = context_->GetSubsystem<FwClient>();
    client->GotoPos(pos);
}

void Player::ClickObject(uint32_t objectId)
{
    FwClient* client = context_->GetSubsystem<FwClient>();
    client->ClickObject(id_, objectId);
}

void Player::SelectObject(uint32_t objectId)
{
    if (objectId == GetSelectedObjectId())
        return;
    FwClient* client = context_->GetSubsystem<FwClient>();
    client->SelectObject(id_, objectId);
}

void Player::PostUpdate(float timeStep)
{
    Node* characterNode = GetNode();
    Shortcuts* scs = GetSubsystem<Shortcuts>();

    float yaw = controls_.yaw_;
    if (scs->Test(AbEvents::E_SC_REVERSECAMERA))
        yaw += 180.0f;
    // Get camera look at dir from character yaw + pitch
    Quaternion rot = Quaternion(yaw, Vector3::UP);
    Quaternion dir = rot * Quaternion(controls_.pitch_, Vector3::RIGHT);

    Node* headNode = characterNode->GetChild("Head", true);

#ifdef PLAYER_HEAD_ANIMATION
    // Turn head to camera pitch, but limit to avoid unnatural animation
    float limitPitch = Clamp(controls_.pitch_, -30.0f, 30.0f);
    float _yaw = controls_.yaw_ - characterNode->GetRotation().YawAngle();
    float yaw = _yaw - floor((_yaw + 180.0f) / 360.0f) * 360.0f;
    float limitYaw = Clamp(yaw, -45.0f, 45.0f);
    Quaternion headDir = characterNode->GetRotation() *
        Quaternion(limitYaw, Vector3::UP) *
        Quaternion(limitPitch, Vector3(1.0f, 0.0f, 0.0f));
    // This could be expanded to look at an arbitrary target, now just look at a point in front
    Vector3 headWorldTarget = headNode->GetWorldPosition() + headDir * Vector3(0.0f, 0.0f, -1.0f);
    headNode->LookAt(headWorldTarget, Vector3(0.0f, 1.0f, 0.0f));
#endif

    // Third person camera: position behind the character
    Vector3 aimPoint;
    if (stickCameraToHead_ && headNode)
        aimPoint = headNode->GetWorldPosition();
    else
    {
        static const Vector3 CAM_POS(0.0f, 1.5f, 0.0f);
        aimPoint = characterNode->GetWorldPosition() + rot * CAM_POS;
    }

    Vector3 rayDir = dir * Vector3::BACK;
    float rayDistance = cameraDistance_;

    if (cameraDistance_ <= 0.2f)
        model_->SetViewMask(0);
    else if (model_->GetViewMask() == 0)
        model_->SetViewMask((unsigned)-1);

    PhysicsRaycastResult result;
    // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
    node_->GetScene()->GetComponent<PhysicsWorld>()->RaycastSingle(result,
        Ray(aimPoint, rayDir), rayDistance, COLLISION_LAYER_CAMERA);
    if (result.body_)
        rayDistance = Min(rayDistance, result.distance_ - 1.0f);
    rayDistance = Clamp(rayDistance, CAMERA_MIN_DIST, CAMERA_MAX_DIST);

    cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);
    cameraNode_->SetRotation(dir);

    Actor::PostUpdate(timeStep);
}

void Player::HandleActorNameClicked(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::ActorNameClicked;
    uint32_t id = eventData[P_SOURCEID].GetUInt();
    if (id != id_)
    {
        ClickObject(id);
        SelectObject(id);
    }
}

void Player::HandleSelectSelf(StringHash, VariantMap&)
{
    SelectObject(id_);
}

