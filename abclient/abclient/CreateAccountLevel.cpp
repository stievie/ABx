#include "stdafx.h"
#include "CreateAccountLevel.h"
#include "AbEvents.h"
#include "FwClient.h"

CreateAccountLevel::CreateAccountLevel(Context* context) :
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

void CreateAccountLevel::DoCreateAccount()
{
    String name = nameEdit_->GetText();
    String pass = passEdit_->GetText();
    String repass = repeatPassEdit_->GetText();
    String email = emailEdit_->GetText();
    String accKey = accKeyEdit_->GetText();
    if (pass.Compare(repass) != 0)
    {
        ShowError("Passwords do not match.");
        repeatPassEdit_->SetFocus(true);
        return;
    }

    FwClient* client = context_->GetSubsystem<FwClient>();
    client->CreateAccount(name, pass, email, accKey);
}

void CreateAccountLevel::DoCancel()
{
    VariantMap& e = GetEventDataMap();
    e[AbEvents::E_SET_LEVEL] = "LoginLevel";
    SendEvent(AbEvents::E_SET_LEVEL, e);
}

void CreateAccountLevel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scene_ = new Scene(context_);
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/CreateAccount.xml");
    scene_->LoadXML(sceneFile->GetRoot());
}

void CreateAccountLevel::CreateCamera()
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

void CreateAccountLevel::ShowError(const String& message, const String& title)
{
    BaseLevel::ShowError(message, title);
    button_->SetEnabled(true);
}

void CreateAccountLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/CreateAccountWindow.xml");
    uiRoot_->LoadChildXML(chatFile->GetRoot());

    nameEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("NameEdit", true));
    passEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("PassEdit", true));
    repeatPassEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("RepeatPassEdit", true));
    emailEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("EmailEdit", true));
    accKeyEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("AccountKeyEdit", true));
    SubscribeToEvent(accKeyEdit_, E_FOCUSED, URHO3D_HANDLER(CreateAccountLevel, HandleAccKeyFocused));
    SubscribeToEvent(accKeyEdit_, E_DEFOCUSED, URHO3D_HANDLER(CreateAccountLevel, HandleAccKeyDefocused));
    accKeyPlaceholder_ = dynamic_cast<Text*>(uiRoot_->GetChild("AccountKeyPlaceHolder", true));
    button_ = dynamic_cast<Button*>(uiRoot_->GetChild("CreateButton", true));
    button_->SetEnabled(false);
    nameEdit_->SetFocus(true);
    SubscribeToEvent(button_, E_RELEASED, URHO3D_HANDLER(CreateAccountLevel, HandleCreateClicked));
    Button* cancelButton = dynamic_cast<Button*>(uiRoot_->GetChild("CancelButton", true));
    SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(CreateAccountLevel, HandleCancelClicked));
}

void CreateAccountLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);

    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Quaternion rot;
    rot.FromAngleAxis(timeStep, Vector3(0.0f, 1.0f, 0.0f));
    cameraNode_->Rotate(rot);
}

void CreateAccountLevel::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;

    int key = eventData[P_KEY].GetInt();
    if (key == KEY_RETURN)
    {
        if (!button_->IsEnabled())
            return;
        DoCreateAccount();
    }
    else if (key == KEY_ESCAPE)
        DoCancel();
}

void CreateAccountLevel::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    String name = nameEdit_->GetText();
    String pass = passEdit_->GetText();
    String repass = repeatPassEdit_->GetText();
    String email = emailEdit_->GetText();
    String accKey = accKeyEdit_->GetText();
    button_->SetEnabled(!name.Empty() && !pass.Empty() && !repass.Empty() && !email.Empty() && !accKey.Empty());
}

void CreateAccountLevel::HandleAccKeyFocused(StringHash eventType, VariantMap& eventData)
{
    accKeyPlaceholder_->SetVisible(false);
}

void CreateAccountLevel::HandleAccKeyDefocused(StringHash eventType, VariantMap& eventData)
{
    String accKey = accKeyEdit_->GetText();
    if (accKey.Empty())
        accKeyPlaceholder_->SetVisible(true);
}

void CreateAccountLevel::HandleCreateClicked(StringHash eventType, VariantMap& eventData)
{
    DoCreateAccount();
}

void CreateAccountLevel::HandleCancelClicked(StringHash eventType, VariantMap& eventData)
{
    DoCancel();
}

void CreateAccountLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CreateAccountLevel, HandleUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(CreateAccountLevel, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(CreateAccountLevel, HandleKeyUp));
}
