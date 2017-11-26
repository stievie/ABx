#include "stdafx.h"
#include "CharSelectLevel.h"
#include "FwClient.h"

CharSelectLevel::CharSelectLevel(Context* context) :
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

void CharSelectLevel::CreatePlayer(const Vector3& position, const Quaternion& direction)
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

void CharSelectLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharSelectLevel, HandleUpdate));
}

void CharSelectLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Window* window_login = new Window(context_);
    uiRoot_->AddChild(window_login);
    window_login->SetSize(400, 300);
    window_login->SetPosition(0, -40);
    window_login->SetLayoutMode(LM_VERTICAL);
    window_login->SetLayoutSpacing(10);
    window_login->SetLayoutBorder(IntRect(10, 10, 10, 10));
    window_login->SetAlignment(HA_CENTER, VA_CENTER);
    window_login->SetName("Select Character");
    window_login->SetStyleAuto();

    FwClient* client = context_->GetSubsystem<FwClient>();
    const Client::Charlist& chars = client->GetCharacters();
    for (const auto& ch : chars)
    {
        Button* button = new Button(context_);
        button->SetMinHeight(40);
        button->SetName("CharacterButton");    // not required
        button->SetStyleAuto();
        button->SetOpacity(1.0f);     // transparency
        button->SetLayoutMode(LM_FREE);
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleCharClicked));
        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = new Text(context_);
            t->SetAlignment(HA_CENTER, VA_CENTER);
            t->SetName("CharacterName");
            t->SetText(String(ch.name.c_str()));
            t->SetStyle("Text");
            button->AddChild(t);
        }
        window_login->AddChild(button);
    }

/*


    */
}

void CharSelectLevel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scene_ = new Scene(context_);
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/CharSelect.xml");
    scene_->LoadXML(sceneFile->GetRoot());
}

void CharSelectLevel::HandleCharClicked(StringHash eventType, VariantMap& eventData)
{
}

void CharSelectLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);

    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Quaternion rot;
    rot.FromAngleAxis(timeStep, Vector3(0.0f, 1.0f, 0.0f));
    cameraNode_->Rotate(rot);
}
