#include "stdafx.h"
#include "CharSelectLevel.h"
#include "FwClient.h"
#include "AudioManager.h"

//#include <Urho3D/DebugNew.h>

CharSelectLevel::CharSelectLevel(Context* context) :
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

void CharSelectLevel::CreateCamera()
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
    SubscribeToEvent(Events::E_ACCOUNTKEYADDED, URHO3D_HANDLER(CharSelectLevel, HandleAccountKeyAdded));
}

void CharSelectLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();

    Window* window = new Window(context_);
    uiRoot_->AddChild(window);
    window->SetSize(400, 300);
    window->SetPosition(0, -40);
    window->SetLayoutMode(LM_VERTICAL);
    window->SetLayoutSpacing(10);
    window->SetLayoutBorder(IntRect(10, 10, 10, 10));
    window->SetAlignment(HA_CENTER, VA_CENTER);
    window->SetName("Select Character");
    window->SetStyleAuto();

    FwClient* client = context_->GetSubsystem<FwClient>();
    const AB::Entities::CharList& chars = client->GetCharacters();
    int i = 0;
    for (const auto& ch : chars)
    {
        Button* button = new Button(context_);
        button->SetMinHeight(40);
        button->SetName(String(ch.uuid.c_str()));    // not required
        button->SetStyleAuto();
        button->SetOpacity(1.0f);     // transparency
        button->SetLayoutMode(LM_FREE);
        button->SetVar("uuid", String(ch.uuid.c_str()));
        button->SetVar("char_name", String(ch.name.c_str()));
        button->SetVar("map_uuid", String(ch.lastOutpostUuid.c_str()));
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleCharClicked));
        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = new Text(context_);
            t->SetAlignment(HA_CENTER, VA_CENTER);
            t->SetName("CharacterName");
            String text = String(ch.profession.c_str());
            if (!ch.profession2.empty())
                text += "/" + String(ch.profession2.c_str());
            text += String((int)ch.level);
            text += " " + String(ch.name.c_str());
            t->SetText(text);
            t->SetStyle("Text");
            button->AddChild(t);
        }
        window->AddChild(button);
        i++;
    }
    {
        if (i != 0)
        {
            UIElement* sep = new UIElement(context_);
            sep->SetMinHeight(5);
            sep->SetStyleAuto();
            sep->SetLayoutMode(LM_FREE);
            window->AddChild(sep);
        }

        Button* button = new Button(context_);
        button->SetMinHeight(40);
        button->SetStyleAuto();
        button->SetOpacity(1.0f);     // transparency
        button->SetLayoutMode(LM_FREE);
        window->AddChild(button);
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleCreateCharClicked));

        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = new Text(context_);
            t->SetAlignment(HA_CENTER, VA_CENTER);
            t->SetText("Create Character");
            t->SetStyle("Text");
            button->AddChild(t);
        }
    }
    {
        Button* button = new Button(context_);
        button->SetMinHeight(40);
        button->SetStyleAuto();
        button->SetOpacity(1.0f);     // transparency
        button->SetLayoutMode(LM_FREE);
        button->SetStyle("BackButton");
        window->AddChild(button);
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleBackClicked));

        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = new Text(context_);
            t->SetAlignment(HA_CENTER, VA_CENTER);
            t->SetText("Back");
            t->SetStyle("Text");
            button->AddChild(t);
        }
    }

    // Add account key button
    auto accKeyButton = uiRoot_->CreateChild<Button>("AddAccountKeyButton");
    accKeyButton->SetStyleAuto();
    accKeyButton->SetLayoutMode(LM_FREE);
    accKeyButton->SetAlignment(HA_LEFT, VA_BOTTOM);
    accKeyButton->SetPosition(8, -8);
    accKeyButton->SetStyleAuto();
    auto accKeyText = accKeyButton->CreateChild<Text>("AddAccountKeyText");
    accKeyText->SetText("Add Account Key");
    accKeyText->SetAlignment(HA_CENTER, VA_CENTER);
    accKeyText->SetStyle("Text");
    accKeyText->SetFontSize(10);
    accKeyButton->SetMinWidth(accKeyText->GetWidth() + 20);
    accKeyButton->SetMinHeight(accKeyText->GetHeight() + 8);
    SubscribeToEvent(accKeyButton, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleAddAccountKeyClicked));
}

void CharSelectLevel::CreateScene()
{
    BaseLevel::CreateScene();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/CharSelect.xml");
    scene_->LoadXML(sceneFile->GetRoot());

    using namespace Events::AudioPlayMapMusic;
    VariantMap& e = GetEventDataMap();
    e[P_MAPUUID] = "SelectCharacter";
    SendEvent(Events::E_AUDIOPLAYMAPMUSIC, e);
}

void CharSelectLevel::HandleCharClicked(StringHash, VariantMap& eventData)
{
    Button* sender = static_cast<Button*>(eventData[Urho3D::Released::P_ELEMENT].GetPtr());
    String uuid = sender->GetVar("uuid").GetString();
    String mapUuid = sender->GetVar("map_uuid").GetString();

    FwClient* net = context_->GetSubsystem<FwClient>();
    net->EnterWorld(uuid, mapUuid);
}

void CharSelectLevel::HandleCreateCharClicked(StringHash, VariantMap&)
{
    VariantMap& e = GetEventDataMap();
    using namespace Events::SetLevel;
    e[P_NAME] = "CharCreateLevel";
    SendEvent(Events::E_SETLEVEL, e);
}

void CharSelectLevel::HandleBackClicked(StringHash, VariantMap&)
{
    VariantMap& e = GetEventDataMap();
    using namespace Events::SetLevel;
    e[P_NAME] = "LoginLevel";
    SendEvent(Events::E_SETLEVEL, e);
}

void CharSelectLevel::HandleAddAccountKeyClicked(StringHash, VariantMap&)
{
    if (!addAccountKeyDialog_)
    {
        addAccountKeyDialog_ = new AddAccountKeyDialog(context_);
        uiRoot_->AddChild(addAccountKeyDialog_);
        addAccountKeyDialog_->SetAlignment(HA_LEFT, VA_BOTTOM);
        addAccountKeyDialog_->SetPosition(8, -8);
    }
    addAccountKeyDialog_->SetVisible(true);
    addAccountKeyDialog_->accountKeyEdit_->SetFocus(true);
}

void CharSelectLevel::HandleAccountKeyAdded(StringHash, VariantMap&)
{
    if (addAccountKeyDialog_)
    {
        addAccountKeyDialog_->SetVisible(false);
        addAccountKeyDialog_->accountKeyEdit_->SetText("");
    }
    using MsgBox = Urho3D::MessageBox;
    /* MsgBox* msgBox = */ new MsgBox(context_, "The key was successfully added to your account",
        "Account key added");
}

void CharSelectLevel::HandleUpdate(StringHash, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Quaternion rot;
    rot.FromAngleAxis(timeStep, Vector3(0.0f, 1.0f, 0.0f));
    cameraNode_->Rotate(rot);
}
