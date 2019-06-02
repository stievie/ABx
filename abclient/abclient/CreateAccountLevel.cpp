#include "stdafx.h"
#include "CreateAccountLevel.h"
#include "AbEvents.h"
#include "FwClient.h"
#include <AB/Entities/Limits.h>
#include "AudioManager.h"

//#include <Urho3D/DebugNew.h>

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
    FwClient* net = context_->GetSubsystem<FwClient>();
    net->SetState(Client::Client::ClientState::CreateAccount);
}

void CreateAccountLevel::DoCreateAccount()
{
    String name = nameEdit_->GetText().Trimmed();
    if (name.Empty())
    {
        ShowError("Please enter a login name for your account.");
        nameEdit_->SetFocus(true);
        return;
    }
    if (name.Length() < 6)
    {
        ShowError("Names must have at least six characters.");
        nameEdit_->SetFocus(true);
        return;
    }
    if (name.Length() > 32)
    {
        ShowError("The name is too long. Max 32 characters allowed.");
        nameEdit_->SetFocus(true);
        return;
    }

    String pass = passEdit_->GetText();
    if (pass.Empty())
    {
        ShowError("Please enter a password for your account.");
        passEdit_->SetFocus(true);
        return;
    }
    if (name.Length() < 6)
    {
        ShowError("Passwords must have at least six characters.");
        passEdit_->SetFocus(true);
        return;
    }

    String email = emailEdit_->GetText().Trimmed();
#if defined(EMAIL_MANDATORY)
    if (email.Empty())
    {
        ShowError("Please enter an Email address.");
        emailEdit_->SetFocus(true);
        return;
    }
#endif

    String repass = repeatPassEdit_->GetText();
    if (pass.Compare(repass) != 0)
    {
        ShowError("Passwords do not match.");
        repeatPassEdit_->SetFocus(true);
        return;
    }

    String accKey = accKeyEdit_->GetText().Trimmed();
    if (accKey.Empty())
    {
        ShowError("Please enter an Account Key.");
        accKeyEdit_->SetFocus(true);
        return;
    }

    FwClient* client = context_->GetSubsystem<FwClient>();
    client->CreateAccount(name, pass, email, accKey);
}

void CreateAccountLevel::DoCancel()
{
    VariantMap& e = GetEventDataMap();
    using namespace AbEvents::SetLevel;
    e[P_NAME] = "LoginLevel";
    SendEvent(AbEvents::E_SETLEVEL, e);
}

void CreateAccountLevel::CreateScene()
{
    BaseLevel::CreateScene();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/CreateAccount.xml");
    scene_->LoadXML(sceneFile->GetRoot());

    using namespace AbEvents::AudioPlayMapMusic;
    VariantMap& e = GetEventDataMap();
    e[P_MAPUUID] = "CreateAccount";
    SendEvent(AbEvents::E_AUDIOPLAYMAPMUSIC, e);
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
    uiRoot_->LoadChildXML(chatFile->GetRoot(), nullptr);

    nameEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("NameEdit", true));
    nameEdit_->SetMaxLength(AB::Entities::Limits::MAX_ACCOUNT_NAME);
    passEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("PassEdit", true));
    repeatPassEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("RepeatPassEdit", true));
    emailEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("EmailEdit", true));
    accKeyEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("AccountKeyEdit", true));
    accKeyEdit_->SetMaxLength(AB::Entities::Limits::MAX_UUID);
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

void CreateAccountLevel::HandleUpdate(StringHash, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Quaternion rot;
    rot.FromAngleAxis(timeStep, Vector3(0.0f, 1.0f, 0.0f));
    cameraNode_->Rotate(rot);
}

void CreateAccountLevel::HandleKeyUp(StringHash, VariantMap& eventData)
{
    using namespace KeyUp;

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

void CreateAccountLevel::HandleKeyDown(StringHash, VariantMap&)
{
    String name = nameEdit_->GetText();
    String pass = passEdit_->GetText();
    String repass = repeatPassEdit_->GetText();
    String accKey = accKeyEdit_->GetText();
#if defined(EMAIL_MANDATORY)
    String email = emailEdit_->GetText();
    button_->SetEnabled(!name.Empty() && !pass.Empty() && !repass.Empty() && !email.Empty() && !accKey.Empty());
#else
    button_->SetEnabled(!name.Empty() && !pass.Empty() && !repass.Empty() && !accKey.Empty());
#endif
}

void CreateAccountLevel::HandleAccKeyFocused(StringHash, VariantMap&)
{
    accKeyPlaceholder_->SetVisible(false);
}

void CreateAccountLevel::HandleAccKeyDefocused(StringHash, VariantMap&)
{
    String accKey = accKeyEdit_->GetText();
    if (accKey.Empty())
        accKeyPlaceholder_->SetVisible(true);
}

void CreateAccountLevel::HandleCreateClicked(StringHash, VariantMap&)
{
    DoCreateAccount();
}

void CreateAccountLevel::HandleCancelClicked(StringHash, VariantMap&)
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
