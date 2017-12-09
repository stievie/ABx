#include "stdafx.h"
#include "BaseLevel.h"
#include "AbEvents.h"
#include "LevelManager.h"
#include <Urho3D/UI/MessageBox.h>
#include "FwClient.h"

void BaseLevel::Run()
{
    if (scene_)
    {
        scene_->SetUpdateEnabled(true);
    }
}

void BaseLevel::Pause()
{
    if (scene_)
    {
        scene_->SetUpdateEnabled(false);
    }
}

void BaseLevel::Dispose()
{
    // Pause the scene, remove all contents from the scene, then remove the scene itself.
    if (scene_)
    {
        scene_->SetUpdateEnabled(false);
        scene_->Clear();
        scene_->Remove();
        scene_ = nullptr;
    }
}

void BaseLevel::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(BaseLevel, HandleUpdate));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(BaseLevel, HandlePostUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(BaseLevel, HandlePostRenderUpdate));
}

void BaseLevel::Update(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);
    UNREFERENCED_PARAMETER(eventData);

    using namespace Update;

    Input* input = GetSubsystem<Input>();

    if (player_)
    {
        // Clear previous controls
        player_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT | CTRL_JUMP, false);

        // Update controls using keys
        UI* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            player_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
            player_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
            player_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
            player_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
            player_->controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));

            if (input->IsMouseGrabbed())
            {
                player_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
                player_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            }

            // Limit pitch
            player_->controls_.pitch_ = Clamp(player_->controls_.pitch_, -80.0f, 80.0f);
            // Set rotation already here so that it's updated every rendering frame instead of every physics frame
//            player_->GetNode()->SetRotation(Quaternion(player_->controls_.yaw_, Vector3::UP));

            // Switch between 1st and 3rd person
            if (input->GetKeyPress(KEY_F))
                firstPerson_ = !firstPerson_;
        }
    }
}

void BaseLevel::ShowError(const String& message, const String& title)
{
    using MsgBox = Urho3D::MessageBox;
    MsgBox* msgBox = new MsgBox(context_, message, title);
}

void BaseLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Update(eventType, eventData);
}

void BaseLevel::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    PostUpdate(eventType, eventData);
}

void BaseLevel::HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData)
{
    PostRenderUpdate(eventType, eventData);
}

void BaseLevel::PostUpdate(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);
    UNREFERENCED_PARAMETER(eventData);

    if (!player_)
        return;

    Node* characterNode = player_->GetNode();

    // Get camera lookat dir from character yaw + pitch
    Quaternion rot = Quaternion(player_->controls_.yaw_, Vector3::UP);
    Quaternion dir = rot * Quaternion(player_->controls_.pitch_, Vector3::RIGHT);

    // Turn head to camera pitch, but limit to avoid unnatural animation
    Node* headNode = characterNode->GetChild("Head", true);
    float limitPitch = Clamp(player_->controls_.pitch_, -45.0f, 45.0f);
    Quaternion headDir = rot * Quaternion(limitPitch, Vector3(1.0f, 0.0f, 0.0f));
    // This could be expanded to look at an arbitrary target, now just look at a point in front
    Vector3 headWorldTarget = headNode->GetWorldPosition() + headDir * Vector3(0.0f, 0.0f, -1.0f);

    if (firstPerson_)
    {
        player_->GetNode()->SetRotation(Quaternion(player_->controls_.yaw_, Vector3::UP));
        cameraNode_->SetPosition(headNode->GetWorldPosition() + rot * Vector3(0.0f, 0.15f, 0.2f));
        cameraNode_->SetRotation(dir);
    }
    else
    {
        // Third person camera: position behind the character
        Vector3 aimPoint = characterNode->GetPosition() + rot * Vector3(0.0f, 2.0f, -3.0f);

        // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
        Vector3 rayDir = dir * Vector3::BACK;
        float rayDistance = CAMERA_INITIAL_DIST;
        PhysicsRaycastResult result;
        PhysicsWorld* world = scene_->GetComponent<PhysicsWorld>();
        world->RaycastSingle(result, Ray(aimPoint, rayDir), rayDistance, 2);
        if (result.body_)
            rayDistance = Min(CAMERA_MIN_DIST, result.distance_);
        rayDistance = Clamp(rayDistance, CAMERA_MIN_DIST, CAMERA_MAX_DIST);
        cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);

        cameraNode_->SetRotation(dir);
    }
}

void BaseLevel::PostRenderUpdate(StringHash eventType, VariantMap & eventData)
{
    UNREFERENCED_PARAMETER(eventType);
    UNREFERENCED_PARAMETER(eventData);

//    if (debugGeometry_)
//        scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
}

void BaseLevel::OnNetworkError(const std::error_code& err)
{
    ShowError(String(err.message().c_str()), "Network Error");
}

void BaseLevel::OnProtocolError(uint8_t err)
{
    String msg = FwClient::GetProtocolErrorMessage(err);
    if (!msg.Empty())
        ShowError(msg, "Error");
}

void BaseLevel::SetupViewport()
{

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    GetSubsystem<Renderer>()->SetViewport(0, viewport);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
    effectRenderPath->SetEnabled("FXAA2", false);
    viewport->SetRenderPath(effectRenderPath);
}

void BaseLevel::CreateLogo()
{
    // Get logo texture
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* logoTexture = cache->GetResource<Texture2D>("Textures/Trill.png");
    if (!logoTexture)
        return;

    // Create logo sprite and add to the UI layout
    UI* ui = GetSubsystem<UI>();
    logoSprite_ = ui->GetRoot()->CreateChild<Sprite>();

    // Set logo sprite texture
    logoSprite_->SetTexture(logoTexture);

    int textureWidth = logoTexture->GetWidth();
    int textureHeight = logoTexture->GetHeight();

    // Set logo sprite scale
    logoSprite_->SetScale(100.0f / textureWidth);

    // Set logo sprite size
    logoSprite_->SetSize(textureWidth, textureHeight);

    // Set logo sprite hot spot
    logoSprite_->SetHotSpot(textureWidth, textureHeight);

    // Set logo sprite alignment
    logoSprite_->SetAlignment(HA_RIGHT, VA_BOTTOM);

    // Make logo not fully opaque to show the scene underneath
    logoSprite_->SetOpacity(0.7f);

    // Set a low priority for the logo so that other UI elements can be drawn on top
    logoSprite_->SetPriority(-100);
}

void BaseLevel::CreatePlayer(const Vector3& position, const Quaternion& direction)
{
/*    Node* objectNode = scene_->CreateChild("Player");
    objectNode->Scale(Vector3(0.01f, 0.01f, 0.01f));
    objectNode->SetPosition(position);
    objectNode->SetRotation(direction);

    // Create the character logic component, which takes care of steering the rigidbody
    // Remember it so that we can set the controls. Use a WeakPtr because the scene hierarchy already owns it
    // and keeps it alive as long as it's not removed from the hierarchy
    player_ = objectNode->CreateComponent<Player>();
    player_->Init();

    cameraNode_ = scene_->CreateChild("CameraNode");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);
    SetupViewport();

    player_->controls_.yaw_ = direction.YawAngle();

    // Add sound listener to camera node
    Node* listenerNode = cameraNode_->CreateChild("SoundListenerNode");
    // Let's face the sound
    listenerNode->SetDirection(Vector3(0.0f, M_HALF_PI, 0.0f));
    SoundListener* soundListener = listenerNode->CreateComponent<SoundListener>();
    GetSubsystem<Audio>()->SetListener(soundListener);          */
}

void BaseLevel::CreatePlayer(const Vector3& position)
{
    CreatePlayer(position, Quaternion());
}

void BaseLevel::CreatePlayer()
{
    Node* spawnPoint = nullptr;
    const String& lastLevel = GetSubsystem<LevelManager>()->GetLastLevelName();
    if (!lastLevel.Empty())
        // Find spawnpoint when we come from a level
        spawnPoint = scene_->GetChild("SpawnPoint" + lastLevel, true);
    if (!spawnPoint)
        // No spawnpoint yet, find default spawn point
        spawnPoint = scene_->GetChild("SpawnPoint", true);
    if (spawnPoint)
    {
        const Vector3& pos = spawnPoint->GetPosition();
        const Quaternion& rot = spawnPoint->GetRotation();
        CreatePlayer(pos, rot);
    }
    else
        CreatePlayer(Vector3(0.0f, 0.0f, 0.0f));
}

void BaseLevel::CreateUI()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* style = cache->GetResource<XMLFile>("UI/FwDefaultStyle.xml");
    uiRoot_->SetDefaultStyle(style);
}
