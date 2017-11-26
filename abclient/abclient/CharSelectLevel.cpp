#include "stdafx.h"
#include "CharSelectLevel.h"


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

/*    button_ = new Button(context_);
    button_->SetLayoutMode(LM_FREE);
    button_->SetMinHeight(40);
    button_->SetName("LoginButton");    // not required
    button_->SetStyleAuto();
    button_->SetOpacity(1.0f);     // transparency

    button_->SetEnabled(false);

    window_login->AddChild(button_);


    SubscribeToEvent(button_, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleCharClicked));
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
}
