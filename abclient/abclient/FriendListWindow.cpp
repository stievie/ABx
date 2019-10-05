#include "stdafx.h"
#include "FriendListWindow.h"
#include "Shortcuts.h"
#include "AbEvents.h"
#include "FwClient.h"

void FriendListWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<FriendListWindow>();
}

FriendListWindow::FriendListWindow(Context* context) :
    Window(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/FriendListWindow.xml");
    LoadXML(file->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetName("FriendListWindow");
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(true);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));
    SetBringToFront(true);
    SetBringToBack(true);

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = dynamic_cast<Text*>(GetChild("Caption", true));
    caption->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLEFRIENDLISTWINDOW, "Friends", true));

    friendList_ = dynamic_cast<ListView*>(GetChild("FriendListView", true));
    ignoreList_ = dynamic_cast<ListView*>(GetChild("IgnoreListView", true));
    addFriendEdit_ = dynamic_cast<LineEdit*>(GetChild("AddFriendEdit", true));
    addIgnoreEdit_ = dynamic_cast<LineEdit*>(GetChild("AddIgnoredEdit", true));

    SetSize(272, 128);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(5, graphics->GetHeight() / 2 + GetHeight());
    SetVisible(true);

    SetStyleAuto();

    SubscribeEvents();
}

FriendListWindow::~FriendListWindow()
{
    UnsubscribeFromAllEvents();
}

void FriendListWindow::GetList()
{
    if (!initialized_)
    {
        auto* client = GetSubsystem<FwClient>();
        client->UpdateFriendList();
//        initialized_ = true;
    }
}

void FriendListWindow::Clear()
{
    friendList_->RemoveAllItems();
    ignoreList_->RemoveAllItems();
    initialized_ = false;
}

void FriendListWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(FriendListWindow, HandleCloseClicked));
    Button* addFriendButton = dynamic_cast<Button*>(GetChild("AddFriendButton", true));
    SubscribeToEvent(addFriendButton, E_RELEASED, URHO3D_HANDLER(FriendListWindow, HandleAddFriendClicked));
    Button* addIgnoreButton = dynamic_cast<Button*>(GetChild("AddIgnoredButton", true));
    SubscribeToEvent(addIgnoreButton, E_RELEASED, URHO3D_HANDLER(FriendListWindow, HandleAddIgnoreClicked));

    SubscribeToEvent(AbEvents::E_GOT_FRIENDLIST, URHO3D_HANDLER(FriendListWindow, HandleGotFriendList));
    SubscribeToEvent(AbEvents::E_PLAYER_LOGGEDOUT, URHO3D_HANDLER(FriendListWindow, HandlePlayerLoggedOut));
    SubscribeToEvent(AbEvents::E_PLAYER_LOGGEDIN, URHO3D_HANDLER(FriendListWindow, HandlePlayerLoggedIn));
}

void FriendListWindow::HandleGotFriendList(StringHash, VariantMap&)
{
    UpdateAll();
}

void FriendListWindow::UpdateAll()
{
    auto* client = GetSubsystem<FwClient>();
    auto& fl = client->GetFriendList();
    friendList_->RemoveAllItems();
    ignoreList_->RemoveAllItems();
    for (const auto& f : fl)
    {
        if (f.relation == AB::Entities::FriendRelationFriend)
        {
            AddItem(friendList_, f);
        }
        else if (f.relation == AB::Entities::FriendRelationIgnore)
        {
            AddItem(ignoreList_, f);
        }
    }
}

void FriendListWindow::AddItem(ListView* lv, const Client::RelatedAccount& f)
{
    Text* txt = lv->CreateChild<Text>();
    txt->SetText(String(f.name.c_str(), static_cast<unsigned>(f.name.length())));
    txt->SetStyle("FriendListItem");
    txt->SetVar("AccountUuid", String(f.accountUuid.c_str()));
    txt->SetVar("CharacterName", String(f.currentName.c_str(), static_cast<unsigned>(f.currentName.length())));
    txt->EnableLayoutUpdate();
    txt->UpdateLayout();
    lv->AddItem(txt);
    lv->EnableLayoutUpdate();
    lv->UpdateLayout();
}

void FriendListWindow::HandlePlayerLoggedOut(StringHash, VariantMap& eventData)
{
}

void FriendListWindow::HandlePlayerLoggedIn(StringHash, VariantMap& eventData)
{
}

void FriendListWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void FriendListWindow::HandleAddFriendClicked(StringHash, VariantMap&)
{
    const String& name = addFriendEdit_->GetText();
    if (name.Empty())
        return;
    auto* client = GetSubsystem<FwClient>();
    client->AddFriend(name, AB::Entities::FriendRelationFriend);
}

void FriendListWindow::HandleAddIgnoreClicked(StringHash, VariantMap&)
{
    const String& name = addIgnoreEdit_->GetText();
    if (name.Empty())
        return;
    auto* client = GetSubsystem<FwClient>();
    client->AddFriend(name, AB::Entities::FriendRelationIgnore);
}
