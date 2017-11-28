#include "stdafx.h"
#include "OutpostLevel.h"


OutpostLevel::OutpostLevel(Context* context) :
    BaseLevel(context)
{
    // Create the scene content
    CreateScene();
    BaseLevel::CreatePlayer();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
}

void OutpostLevel::CreatePlayer(const Vector3& position, const Quaternion& direction)
{
    cameraNode_ = scene_->GetChild("CameraNode");
    if (!cameraNode_)
    {
        cameraNode_ = scene_->CreateChild("CameraNode");
        Camera* camera = cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(300.0f);
    }
    SetupViewport();
}

void OutpostLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(OutpostLevel, HandleUpdate));
}

void OutpostLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();
}

void OutpostLevel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scene_ = new Scene(context_);
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/CharSelect.xml");
    scene_->LoadXML(sceneFile->GetRoot());
}

void OutpostLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
}
