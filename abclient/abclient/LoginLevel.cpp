#include "stdafx.h"
#include "LoginLevel.h"
#include "AbEvents.h"
#include "FwClient.h"
#include "Options.h"

LoginLevel::LoginLevel(Context* context) :
    BaseLevel(context),
    loggingIn_(false)
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
    net->SetState(Client::Client::StateDisconnected);
}

void LoginLevel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scene_ = new Scene(context_);
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/Login.xml");
    scene_->LoadXML(sceneFile->GetRoot());
}

void LoginLevel::CreateCamera()
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

void LoginLevel::ShowError(const String& message, const String& title)
{
    BaseLevel::ShowError(message, title);
    button_->SetEnabled(true);
    passEdit_->SetFocus(true);
    loggingIn_ = false;
}

void LoginLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/LoginWindow.xml");
    uiRoot_->LoadChildXML(chatFile->GetRoot());

    nameEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("NameEdit", true));
    passEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("PassEdit", true));
    button_ = dynamic_cast<Button*>(uiRoot_->GetChild("LoginButton", true));
    button_->SetEnabled(false);
    nameEdit_->SetFocus(true);
    SubscribeToEvent(button_, E_RELEASED, URHO3D_HANDLER(LoginLevel, HandleLoginClicked));
    createAccountButton_ = dynamic_cast<Button*>(uiRoot_->GetChild("CreateAccountButton", true));
    SubscribeToEvent(createAccountButton_, E_RELEASED, URHO3D_HANDLER(LoginLevel, HandleCreateAccountClicked));

    Options* options = GetSubsystem<Options>();
    nameEdit_->SetText(options->username_);
    passEdit_->SetText(options->password_);
    button_->SetEnabled(!(options->username_.Empty() || options->password_.Empty()));
}

void LoginLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);

    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Quaternion rot;
    rot.FromAngleAxis(timeStep, Vector3(0.0f, 1.0f, 0.0f));
    cameraNode_->Rotate(rot);
}

void LoginLevel::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    if (loggingIn_)
        return;

    using namespace KeyDown;
    if (!button_->IsEnabled())
        return;
    String name = nameEdit_->GetText();
    String pass = passEdit_->GetText();
    if (name.Empty() || pass.Empty())
        return;

    int key = eventData[P_KEY].GetInt();
    if (key == KEY_RETURN)
        DoLogin();
}

void LoginLevel::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    String name = nameEdit_->GetText();
    String pass = passEdit_->GetText();
    button_->SetEnabled(!name.Empty() && !pass.Empty());
}

void LoginLevel::DoLogin()
{
    if (loggingIn_)
        return;

    loggingIn_ = true;
    button_->SetEnabled(false);
    String name = nameEdit_->GetText();
    String pass = passEdit_->GetText();
    FwClient* net = context_->GetSubsystem<FwClient>();
    net->Login(name, pass);
}

void LoginLevel::HandleLoginClicked(StringHash eventType, VariantMap& eventData)
{
    DoLogin();
}

void LoginLevel::HandleCreateAccountClicked(StringHash eventType, VariantMap& eventData)
{
    VariantMap& e = GetEventDataMap();
    e[AbEvents::E_SET_LEVEL] = "CreateAccountLevel";
    SendEvent(AbEvents::E_SET_LEVEL, e);
}

void LoginLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LoginLevel, HandleUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(LoginLevel, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(LoginLevel, HandleKeyUp));
}
