/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "LoginLevel.h"
#include "FwClient.h"
#include "Options.h"
#include <AB/Entities/Limits.h>
#include "WindowManager.h"
#include "PartyWindow.h"
#include "MultiLineEdit.h"

//#include <Urho3D/DebugNew.h>

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
    FwClient* net = GetSubsystem<FwClient>();
    net->SetState(Client::State::Disconnected);
}

void LoginLevel::CreateScene()
{
    BaseLevel::CreateScene();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
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
    passEdit_->GetTextElement()->SetSelection(0);
    loggingIn_ = false;
}

void LoginLevel::CreateEnvironmentsList()
{
    Options* opts = GetSubsystem<Options>();
    const auto& envs = opts->environments_;
    if (envs.Size() != 0)
    {
        environmentsList_ = uiRoot_->CreateChild<DropDownList>("Environments");
        environmentsList_->SetStyleAuto();
        environmentsList_->SetAlignment(HA_LEFT, VA_BOTTOM);
        environmentsList_->SetPosition(8, -8);
        // Make the popup same width as the control
        environmentsList_->SetResizePopup(true);

        Environment* selEnv = opts->GetSelectedEnvironment();
        int width = 0; int height = 0;
        unsigned selIndex = 0;
        unsigned i = 0;
        for (const auto& env : envs)
        {
            Text* txt = CreateDropdownItem(env.name, env.name);
            environmentsList_->AddItem(txt);
            if (width < txt->GetWidth())
                width = txt->GetWidth();
            if (height < txt->GetHeight())
                height = txt->GetHeight();
            if (selEnv != nullptr)
            {
                if (selEnv->name.Compare(env.name) == 0)
                {
                    selIndex = i;
                }
            }
            ++i;
        }
        environmentsList_->SetMinWidth(width + 50);
        environmentsList_->SetWidth(width + 50);
        environmentsList_->SetMinHeight(height + 4);
        environmentsList_->SetHeight(height + 4);
        environmentsList_->SetSelection(selIndex);
    }
}

void LoginLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();

    WindowManager* wm = GetSubsystem<WindowManager>();
    PartyWindow* partyWindow = dynamic_cast<PartyWindow*>(wm->GetWindow(WINDOW_PARTY).Get());
    partyWindow->Clear();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/LoginWindow.xml");
    uiRoot_->LoadChildXML(chatFile->GetRoot(), nullptr);

    nameEdit_ = uiRoot_->GetChildStaticCast<LineEdit>("NameEdit", true);
    nameEdit_->SetMaxLength(AB::Entities::Limits::MAX_ACCOUNT_NAME);
    passEdit_ = uiRoot_->GetChildStaticCast<LineEdit>("PassEdit", true);
    button_ = uiRoot_->GetChildStaticCast<Button>("LoginButton", true);
    button_->SetEnabled(false);
    nameEdit_->SetFocus(true);
    SubscribeToEvent(button_, E_RELEASED, URHO3D_HANDLER(LoginLevel, HandleLoginClicked));

    SubscribeToEvent(nameEdit_, E_TEXTFINISHED, URHO3D_HANDLER(LoginLevel, HandleTextFinished));
    SubscribeToEvent(passEdit_, E_TEXTFINISHED, URHO3D_HANDLER(LoginLevel, HandleTextFinished));

    createAccountButton_ = uiRoot_->GetChildStaticCast<Button>("CreateAccountButton", true);
    SubscribeToEvent(createAccountButton_, E_RELEASED, URHO3D_HANDLER(LoginLevel, HandleCreateAccountClicked));

    FwClient* net = GetSubsystem<FwClient>();
    if (!net->accountName_.Empty() && !net->accountPass_.Empty())
    {
        nameEdit_->SetText(net->accountName_);
        passEdit_->SetText(net->accountPass_);
    }
    else
    {
        Options* options = GetSubsystem<Options>();
        nameEdit_->SetText(options->username_);
        passEdit_->SetText(options->password_);
    }
    button_->SetEnabled(!(nameEdit_->GetText().Empty() || passEdit_->GetText().Empty()));

#if 0
    auto e = uiRoot_->CreateChild<MultiLineEdit>();
    e->SetPosition({ 0, 0 });
    e->SetSize({ 200, 200 });
    e->SetMinSize({ 200, 200 });
    e->SetAlignment(HA_LEFT, VA_TOP);
    e->SetStyleAuto();
    e->SetClipBorder({ 4, 4, 4, 4 });
    e->SetWordwrap(true);
//    e->SetEnableLinebreak(false);
#endif

    CreateEnvironmentsList();
}

Text* LoginLevel::CreateDropdownItem(const String& text, const String& value)
{
    Text* result = new Text(context_);
    result->SetText(text);
    result->SetVar("String Value", value);
    result->SetStyle("DropDownItemEnumText");
    return result;
}

void LoginLevel::HandleUpdate(StringHash, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Quaternion rot;
    rot.FromAngleAxis(timeStep, Vector3(0.0f, 1.0f, 0.0f));
    cameraNode_->Rotate(rot);
}

void LoginLevel::HandleTextFinished(StringHash, VariantMap&)
{
    if (loggingIn_)
        return;

    if (!button_->IsEnabled())
        return;
    String name = nameEdit_->GetText();
    String pass = passEdit_->GetText();
    if (name.Empty() || pass.Empty())
        return;

    DoLogin();
}

void LoginLevel::HandleKeyDown(StringHash, VariantMap&)
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
    Options* opts = GetSubsystem<Options>();
    auto& envs = opts->environments_;
    unsigned selEnv = environmentsList_->GetSelection();
    FwClient* net = GetSubsystem<FwClient>();
    if (selEnv < envs.Size())
    {
        Environment* env = &envs[selEnv];
        net->SetEnvironment(env);
        opts->SetSelectedEnvironment(env->name);
    }
    net->Login(name, pass);
}

void LoginLevel::HandleLoginClicked(StringHash, VariantMap&)
{
    DoLogin();
}

void LoginLevel::HandleCreateAccountClicked(StringHash, VariantMap&)
{
    VariantMap& e = GetEventDataMap();
    using namespace Events::SetLevel;
    e[P_NAME] = "CreateAccountLevel";
    SendEvent(Events::E_SETLEVEL, e);
}

void LoginLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LoginLevel, HandleUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(LoginLevel, HandleKeyDown));
}
