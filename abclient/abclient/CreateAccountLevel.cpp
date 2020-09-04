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

#include "CreateAccountLevel.h"
#include "FwClient.h"
#include <AB/Entities/Limits.h>
#include "AudioManager.h"

//#include <Urho3D/DebugNew.h>

CreateAccountLevel::CreateAccountLevel(Context* context) :
    BaseLevel(context)
{
    // Create the scene content
    CreateScene();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
    FwClient* net = GetSubsystem<FwClient>();
    net->SetState(Client::State::CreateAccount);
}

void CreateAccountLevel::SceneLoadingFinished()
{
    CreateCamera();

    // Create the UI content
    CreateUI();
    CreateLogo();

    VariantMap& eData = GetEventDataMap();
    using namespace Events::LevelReady;
    eData[P_NAME] = "CreateAccountLevel";
    eData[P_TYPE] = 0;
    SendEvent(Events::E_LEVELREADY, eData);
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
    if (name.Length() < ACCOUNT_NAME_MIN)
    {
        ShowError("Names must have at least six characters.");
        nameEdit_->SetFocus(true);
        return;
    }
    if (name.Length() > ACCOUNT_NAME_MAX)
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
    if (pass.Length() < PASSWORD_LENGTH_MIN)
    {
        ShowError("Passwords must have at least six characters.");
        passEdit_->SetFocus(true);
        return;
    }
    if (pass.Length() > PASSWORD_LENGTH_MAX)
    {
        ShowError("Password is too long. Max 61 characters allowed.");
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
    if (email.Length() < 3)
    {
        ShowError("Please enter a valid Email address.");
        emailEdit_->SetFocus(true);
        return;
    }
    if (email.Length() > EMAIL_LENGTH_MAX)
    {
        ShowError("Email address is too long.");
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

    FwClient* client = GetSubsystem<FwClient>();
    client->CreateAccount(name, pass, email, accKey);
}

void CreateAccountLevel::DoCancel()
{
    VariantMap& e = GetEventDataMap();
    using namespace Events::SetLevel;
    e[P_NAME] = "LoginLevel";
    SendEvent(Events::E_SETLEVEL, e);
}

void CreateAccountLevel::CreateScene()
{
    BaseLevel::CreateScene();
    LoadScene("Scenes/CreateAccount.xml");

    using namespace Events::AudioPlayMapMusic;
    VariantMap& e = GetEventDataMap();
    e[P_MAPUUID] = "CreateAccount";
    SendEvent(Events::E_AUDIOPLAYMAPMUSIC, e);
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
    BaseLevel::CreateUI();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/CreateAccountWindow.xml");
    uiRoot_->LoadChildXML(chatFile->GetRoot(), nullptr);

    nameEdit_ = uiRoot_->GetChildStaticCast<LineEdit>("NameEdit", true);
    nameEdit_->SetMaxLength(AB::Entities::Limits::MAX_ACCOUNT_NAME);
    passEdit_ = uiRoot_->GetChildStaticCast<LineEdit>("PassEdit", true);
    repeatPassEdit_ = uiRoot_->GetChildStaticCast<LineEdit>("RepeatPassEdit", true);
    emailEdit_ = uiRoot_->GetChildStaticCast<LineEdit>("EmailEdit", true);
    accKeyEdit_ = uiRoot_->GetChildStaticCast<LineEdit>("AccountKeyEdit", true);
    accKeyEdit_->SetMaxLength(AB::Entities::Limits::MAX_UUID);
    SubscribeToEvent(accKeyEdit_, E_FOCUSED, URHO3D_HANDLER(CreateAccountLevel, HandleAccKeyFocused));
    SubscribeToEvent(accKeyEdit_, E_DEFOCUSED, URHO3D_HANDLER(CreateAccountLevel, HandleAccKeyDefocused));
    accKeyPlaceholder_ = uiRoot_->GetChildStaticCast<Text>("AccountKeyPlaceHolder", true);
    button_ = uiRoot_->GetChildStaticCast<Button>("CreateButton", true);
    button_->SetEnabled(false);
    nameEdit_->SetFocus(true);
    SubscribeToEvent(button_, E_RELEASED, URHO3D_HANDLER(CreateAccountLevel, HandleCreateClicked));
    Button* cancelButton = uiRoot_->GetChildStaticCast<Button>("CancelButton", true);
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
