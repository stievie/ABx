#include "stdafx.h"
#include "CharCreateLevel.h"

CharCreateLevel::CharCreateLevel(Context* context) :
    BaseLevel(context)
{
    // Create the scene content
    CreateScene();
    CreateCamera();

    // Create the UI content
    CreateUI();
    CreateLogo();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
}

void CharCreateLevel::CreateCamera()
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

void CharCreateLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharCreateLevel, HandleUpdate));
}

void CharCreateLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();
}

void CharCreateLevel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scene_ = new Scene(context_);
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/CreateCharacter.xml");
    scene_->LoadXML(sceneFile->GetRoot());
}

void CharCreateLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);

    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Quaternion rot;
    rot.FromAngleAxis(timeStep, Vector3(0.0f, 1.0f, 0.0f));
    cameraNode_->Rotate(rot);
}
