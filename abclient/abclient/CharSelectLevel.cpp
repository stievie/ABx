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


#include "CharSelectLevel.h"
#include "FwClient.h"
#include "AudioManager.h"
#include "ShortcutEvents.h"
#include "ConfirmDeleteCharacter.h"
#include "ServerEvents.h"
#include "Conversions.h"
#include <sa/time.h>

//#include <Urho3D/DebugNew.h>

CharSelectLevel::CharSelectLevel(Context* context) :
    BaseLevel(context)
{
    // Create the scene content
    CreateScene();

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
    SubscribeToEvent(Events::E_CHARACTERDELETED, URHO3D_HANDLER(CharSelectLevel, HandleCharacterDeleted));
}

void CharSelectLevel::CreateUI()
{
    BaseLevel::CreateUI();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* deleteTexture = cache->GetResource<Texture2D>("Textures/Icons/delete_button.png");

    characterWindow_ = uiRoot_->CreateChild<Window>();
    characterWindow_->SetSize(320, 300);
    characterWindow_->SetPosition(-150, -40);
    characterWindow_->SetLayoutMode(LM_VERTICAL);
    characterWindow_->SetLayoutSpacing(10);
    characterWindow_->SetLayoutBorder(IntRect(10, 10, 10, 10));
    characterWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    characterWindow_->SetName("Select Character");
    characterWindow_->SetStyleAuto();

    FwClient* client = GetSubsystem<FwClient>();
    const AB::Entities::CharList& chars = client->GetCharacters();
    int i = 0;
    for (const auto& ch : chars)
    {
        auto* container = characterWindow_->CreateChild<UIElement>(String(ch.uuid.c_str()));
        container->SetLayoutMode(LM_HORIZONTAL);
        container->SetMinHeight(40);
        container->SetMaxHeight(40);
        container->SetLayoutSpacing(8);

        Button* button = container->CreateChild<Button>("CharacterButton");
        button->SetStyleAuto();
        button->SetOpacity(1.0f);
        button->SetLayoutMode(LM_FREE);
        button->SetVar("uuid", String(ch.uuid.c_str()));
        button->SetVar("char_name", String(ch.name.c_str()));
        button->SetVar("map_uuid", String(ch.lastOutpostUuid.c_str()));
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleCharClicked));
        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = button->CreateChild<Text>();
            t->SetAlignment(HA_CENTER, VA_CENTER);
            t->SetName("CharacterName");
            String text = String(ch.profession.c_str());
            if (!ch.profession2.empty())
                text += "/" + String(ch.profession2.c_str());
            text += String(static_cast<int>(ch.level));
            text += " " + String(ch.name.c_str());
            t->SetText(text);
            t->SetStyle("Text");
        }

        Button* deleteButton = container->CreateChild<Button>("CharacterDeleteButton");
        deleteButton->SetMinWidth(40);
        deleteButton->SetMaxWidth(40);
        deleteButton->SetStyleAuto();
        deleteButton->SetLayout(LM_HORIZONTAL);
        deleteButton->SetLayoutBorder({ 4, 4, 4, 4 });
        deleteButton->SetVar("uuid", String(ch.uuid.c_str()));
        deleteButton->SetVar("char_name", String(ch.name.c_str()));
        auto* deleteButtonIcon = deleteButton->CreateChild<BorderImage>();
        deleteButtonIcon->SetTexture(deleteTexture);
        deleteButtonIcon->SetImageRect(IntRect(0, 0, 256, 256));
        deleteButtonIcon->SetBorder(IntRect(4, 4, 4, 4));
        SubscribeToEvent(deleteButton, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleDeleteCharClicked));

        i++;
    }
    {
        if (i != 0)
        {
            UIElement* sep = characterWindow_->CreateChild<UIElement>();
            sep->SetMinHeight(5);
            sep->SetMaxHeight(5);
            sep->SetStyleAuto();
            sep->SetLayoutMode(LM_FREE);
        }

        Button* button = characterWindow_->CreateChild<Button>();
        button->SetMinHeight(40);
        button->SetMaxHeight(40);
        button->SetStyleAuto();
        button->SetOpacity(1.0f);
        button->SetLayoutMode(LM_FREE);
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleCreateCharClicked));

        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = button->CreateChild<Text>();
            t->SetAlignment(HA_CENTER, VA_CENTER);
            t->SetText("Create Character");
            t->SetStyle("Text");
        }
    }
    {
        Button* button = characterWindow_->CreateChild<Button>();
        button->SetMinHeight(40);
        button->SetMaxHeight(40);
        button->SetStyleAuto();
        button->SetOpacity(1.0f);     // transparency
        button->SetLayoutMode(LM_FREE);
        button->SetStyle("BackButton");
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleBackClicked));

        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = button->CreateChild<Text>();
            t->SetAlignment(HA_CENTER, VA_CENTER);
            t->SetText("Back");
            t->SetStyle("Text");
            button->AddChild(t);
        }
    }

    auto* buttonsContainer = uiRoot_->CreateChild<UIElement>();
    buttonsContainer->SetLayoutMode(LM_HORIZONTAL);
    buttonsContainer->SetAlignment(HA_LEFT, VA_BOTTOM);
    buttonsContainer->SetLayoutBorder({ 4, 4, 4, 4 });
    buttonsContainer->SetLayoutSpacing(4);
    buttonsContainer->SetPosition(8, -8);

    // Add account key button
    auto* accKeyButton = buttonsContainer->CreateChild<Button>("AddAccountKeyButton");
    accKeyButton->SetPosition(0, 0);
    accKeyButton->SetStyleAuto();
    auto* accKeyText = accKeyButton->CreateChild<Text>("AddAccountKeyText");
    accKeyText->SetText("Add Account Key...");
    accKeyText->SetAlignment(HA_CENTER, VA_CENTER);
    accKeyText->SetStyleAuto();
    accKeyText->SetFontSize(9);
    SubscribeToEvent(accKeyButton, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleAddAccountKeyClicked));

    auto* optionsKeyButton = buttonsContainer->CreateChild<Button>();
    optionsKeyButton->SetPosition(0, 0);
    optionsKeyButton->SetStyleAuto();
    auto* optionsKeyText = optionsKeyButton->CreateChild<Text>();
    optionsKeyText->SetText("Options");
    optionsKeyText->SetAlignment(HA_CENTER, VA_CENTER);
    optionsKeyText->SetStyleAuto();
    optionsKeyText->SetFontSize(9);
    SubscribeToEvent(optionsKeyButton, E_RELEASED, URHO3D_HANDLER(CharSelectLevel, HandleOptionsClicked));

    buttonsContainer->SetHeight(30);
    buttonsContainer->SetWidth(300);

    const auto& news = client->GetNews();
    if (news.size() != 0)
    {
        auto* newsWindow = uiRoot_->CreateChild<Window>();
        newsWindow->SetSize(300, 400);
        newsWindow->SetMinSize(300, 400);
        newsWindow->SetMaxSize(300, 500);
        newsWindow->SetPosition(230, -40);
        newsWindow->SetLayoutMode(LM_VERTICAL);
        newsWindow->SetLayoutBorder(IntRect(4, 4, 4, 4));
        newsWindow->SetAlignment(HA_CENTER, VA_CENTER);
        newsWindow->SetName("News");
        newsWindow->SetStyleAuto();

        newsWindow->SetLayoutMode(LM_VERTICAL);
        auto* newsList = newsWindow->CreateChild<ListView>();
        newsList->SetStyleAuto();
        newsList->SetLayoutBorder(IntRect(4, 4, 4, 4));
        for (const auto& n : news)
        {
            auto* newsContainer = newsList->CreateChild<UIElement>();
            newsContainer->SetStyleAuto();
            newsContainer->SetLayoutMode(LM_VERTICAL);
            newsContainer->SetLayoutBorder(IntRect(4, 4, 4, 4));
            newsContainer->SetLayoutSpacing(4);
            auto* newsDate = newsContainer->CreateChild<Text>();
            newsDate->SetText(ToUrhoString(sa::time::format_tick(n.created)));
            newsDate->SetStyleAuto();
            newsDate->SetFontSize(8);
            auto* newsText = newsContainer->CreateChild<Text>();
            newsText->SetStyleAuto();
            newsText->SetFontSize(9);
            newsText->SetWordwrap(true);
            newsText->SetText(ToUrhoString(n.body));
            newsList->AddItem(newsContainer);
            newsContainer->SetMaxHeight(newsText->GetHeight() + 20);
            newsContainer->SetMinHeight(newsText->GetHeight() + 20);
        }
        newsWindow->UpdateLayout();
    }
}

void CharSelectLevel::CreateScene()
{
    BaseLevel::CreateScene();
    LoadScene("Scenes/CharSelect.xml");

    using namespace Events::AudioPlayMapMusic;
    VariantMap& e = GetEventDataMap();
    e[P_MAPUUID] = "SelectCharacter";
    SendEvent(Events::E_AUDIOPLAYMAPMUSIC, e);
}

void CharSelectLevel::SceneLoadingFinished()
{
    CreateCamera();

    // Create the UI content
    CreateUI();
    CreateLogo();

    VariantMap& eData = GetEventDataMap();
    using namespace Events::LevelReady;
    eData[P_NAME] = "CharSelectLevel";
    eData[P_TYPE] = 0;
    SendEvent(Events::E_LEVELREADY, eData);
}

void CharSelectLevel::HandleCharClicked(StringHash, VariantMap& eventData)
{
    using namespace Urho3D::Released;
    EnableButtons(false);
    Button* sender = static_cast<Button*>(eventData[P_ELEMENT].GetPtr());
    String uuid = sender->GetVar("uuid").GetString();
    String mapUuid = sender->GetVar("map_uuid").GetString();

    FwClient* net = GetSubsystem<FwClient>();
    net->EnterWorld(uuid, mapUuid);
}

void CharSelectLevel::HandleDeleteCharClicked(StringHash, VariantMap& eventData)
{
    using namespace Urho3D::Released;
    Button* sender = static_cast<Button*>(eventData[P_ELEMENT].GetPtr());
    String uuid = sender->GetVar("uuid").GetString();
    String name = sender->GetVar("char_name").GetString();

    new ConfirmDeleteCharacter(context_, uuid, name);
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
    auto* dialog = new AddAccountKeyDialog(context_);
    dialog->Center();
    dialog->SetVisible(true);
    dialog->accountKeyEdit_->SetFocus(true);
}

void CharSelectLevel::HandleOptionsClicked(StringHash, VariantMap&)
{
    VariantMap& e = GetEventDataMap();
    SendEvent(Events::E_SC_TOGGLEOPTIONS, e);
}

void CharSelectLevel::HandleAccountKeyAdded(StringHash, VariantMap&)
{
    using MsgBox = Urho3D::MessageBox;
    /* MsgBox* msgBox = */ new MsgBox(context_, "The key was successfully added to your account",
        "Account key added");
}

void CharSelectLevel::HandleCharacterDeleted(StringHash, VariantMap& eventData)
{
    using namespace Events::CharacterDeleted;
    const String& uuid = eventData[P_UUID].GetString();

    auto* container = characterWindow_->GetChild(uuid, true);
    if (!container)
        return;

    container->Remove();
    characterWindow_->UpdateLayout();
}

void CharSelectLevel::EnableButtons(bool enable)
{
    const auto& children = characterWindow_->GetChildren();

    for (auto childIt = children.Begin(); childIt != children.End(); childIt++)
    {
        auto* characterButton = (*childIt)->GetChildDynamicCast<Button>("CharacterButton", true);
        if (characterButton)
            characterButton->SetEnabled(enable);
        auto* deleteButton = (*childIt)->GetChildDynamicCast<Button>("CharacterDeleteButton", true);
        if (deleteButton)
            deleteButton->SetEnabled(enable);
    }
}

void CharSelectLevel::HandleUpdate(StringHash, VariantMap& eventData)
{
    if (!cameraNode_)
        return;
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    const Quaternion rot{ timeStep, Vector3::UP };
    cameraNode_->Rotate(rot);
}
