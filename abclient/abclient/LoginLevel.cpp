#include "stdafx.h"
#include "LoginLevel.h"
#include "AbEvents.h"
#include "FwClient.h"

LoginLevel::LoginLevel(Context* context) :
    BaseLevel(context),
    loggingIn_(false)
{
    // Create the scene content
    CreateScene();
    BaseLevel::CreatePlayer();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
}

void LoginLevel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scene_ = new Scene(context_);
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/Login.xml");
    scene_->LoadXML(sceneFile->GetRoot());
}

void LoginLevel::CreatePlayer(const Vector3& position, const Quaternion& direction)
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
    loggingIn_ = false;
}

void LoginLevel::CreateUI()
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
    window_login->SetName("Login");
    window_login->SetStyleAuto();

    {
        Text* t = new Text(context_);
        t->SetAlignment(HA_CENTER, VA_TOP);
        t->SetName("NameText");
        t->SetText("Name");
        t->SetStyle("Text");
        window_login->AddChild(t);
    }
    nameEdit_ = new LineEdit(context_);
    nameEdit_->SetName("NameEdit");
    nameEdit_->SetMinHeight(30);
    nameEdit_->SetStyleAuto();
    nameEdit_->SetCursorBlinkRate(1.2f);
    window_login->AddChild(nameEdit_);

    {
        Text* t = new Text(context_);
        t->SetAlignment(HA_CENTER, VA_TOP);
        t->SetName("PassText");
        t->SetText("Password");
        t->SetStyle("Text");
        window_login->AddChild(t);
    }
    passEdit_ = new LineEdit(context_);
    passEdit_->SetName("PassEdit");
    passEdit_->SetMinHeight(30);
    passEdit_->SetStyleAuto();
    passEdit_->SetEchoCharacter('*');
    passEdit_->SetCursorBlinkRate(1.0f);
    window_login->AddChild(passEdit_);

    button_ = new Button(context_);
    button_->SetLayoutMode(LM_FREE);
    button_->SetMinHeight(40);
    button_->SetName("LoginButton");    // not required
    button_->SetStyleAuto();
    button_->SetOpacity(1.0f);     // transparency
    {
        // buttons don't have a text by itself, a text needs to be added as a child
        Text* t = new Text(context_);
        t->SetAlignment(HA_CENTER, VA_CENTER);
        t->SetName("LoginText");
        t->SetText("Login");
        t->SetStyle("Text");
        button_->AddChild(t);
    }
    button_->SetEnabled(false);

    window_login->AddChild(button_);

    nameEdit_->SetFocus(true);

    SubscribeToEvent(button_, E_RELEASED, URHO3D_HANDLER(LoginLevel, HandleLoginClicked));
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
    String name = nameEdit_->GetText();
    String pass = passEdit_->GetText();
    button_->SetEnabled(!name.Empty() && !pass.Empty());
}

void LoginLevel::HandleKeyDown(StringHash eventType, VariantMap& eventData)
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

void LoginLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LoginLevel, HandleUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(LoginLevel, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(LoginLevel, HandleKeyUp));
}
