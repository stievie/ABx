#include "stdafx.h"
#include "FriendListWindow.h"
#include "Shortcuts.h"
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
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLEFRIENDLISTWINDOW, "Friends", true));

    friendList_ = dynamic_cast<ListView*>(GetChild("FriendListView", true));
    ignoreList_ = dynamic_cast<ListView*>(GetChild("IgnoreListView", true));
    addFriendEdit_ = dynamic_cast<LineEdit*>(GetChild("AddFriendEdit", true));
    addIgnoreEdit_ = dynamic_cast<LineEdit*>(GetChild("AddIgnoredEdit", true));

    SetSize(272, 128);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(5, graphics->GetHeight() / 2 + GetHeight());
    SetVisible(true);

    SetStyleAuto();

    friendPopup_ = new Menu(context_);
    friendPopup_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    friendPopup_->SetStyleAuto();
    Window* popup = friendPopup_->CreateChild<Window>();
    popup->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    popup->SetStyle("MenuPopupWindow");
    popup->SetLayout(LM_VERTICAL, 1);
    friendPopup_->SetPopup(popup);

    int width = 0;
    int height = 0;

    {
        // Whisper
        Menu* item = popup->CreateChild<Menu>();
        item->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
        item->SetStyleAuto();
        Text* menuText = item->CreateChild<Text>();
        menuText->SetText("Whisper");
        menuText->SetStyle("EditorMenuText");
        item->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
        item->SetMinSize(menuText->GetSize() + IntVector2(4, 4));
        item->SetSize(item->GetMinSize());
        if (item->GetWidth() > width)
            width = item->GetWidth();
        if (item->GetHeight() > height)
            height = item->GetHeight();
        SubscribeToEvent(item, E_MENUSELECTED, URHO3D_HANDLER(FriendListWindow, HandleFriendWhisperClicked));
    }
    {
        // Remove
        Menu* item = popup->CreateChild<Menu>();
        item->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
        item->SetStyleAuto();
        Text* menuText = item->CreateChild<Text>();
        menuText->SetText("Remove");
        menuText->SetStyle("EditorMenuText");
        item->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
        item->SetMinSize(menuText->GetSize() + IntVector2(4, 4));
        item->SetSize(item->GetMinSize());
        if (item->GetWidth() > width)
            width = item->GetWidth();
        if (item->GetHeight() > height)
            height = item->GetHeight();
        SubscribeToEvent(item, E_MENUSELECTED, URHO3D_HANDLER(FriendListWindow, HandleFriendRemoveClicked));
    }
    popup->SetMinSize(IntVector2(width, height));
    popup->SetSize(popup->GetMinSize());
    friendPopup_->SetSize(popup->GetSize());

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

    SubscribeToEvent(Events::E_GOT_PLAYERINFO, URHO3D_HANDLER(FriendListWindow, HandleGotPlayerInfo));
    SubscribeToEvent(Events::E_GOT_FRIENDLIST, URHO3D_HANDLER(FriendListWindow, HandleGotFriendList));
    SubscribeToEvent(Events::E_FRIENDADDED, URHO3D_HANDLER(FriendListWindow, HandleFriendAdded));
    SubscribeToEvent(Events::E_FRIENDREMOVED, URHO3D_HANDLER(FriendListWindow, HandleFriendRemoved));
    SubscribeToEvent(Events::E_PLAYER_LOGGEDOUT, URHO3D_HANDLER(FriendListWindow, HandlePlayerLoggedOut));
    SubscribeToEvent(Events::E_PLAYER_LOGGEDIN, URHO3D_HANDLER(FriendListWindow, HandlePlayerLoggedIn));
}

void FriendListWindow::HandleGotFriendList(StringHash, VariantMap&)
{
    UpdateAll();
}

void FriendListWindow::UpdateAll()
{
    auto* client = GetSubsystem<FwClient>();
    const auto& fl = client->GetFriendList();
    friendList_->RemoveAllItems();
    ignoreList_->RemoveAllItems();
    for (const auto& f : fl)
    {
        // Should trigger GotPlayerInfo where we add the item
        client->GetPlayerInfoByAccount(f);
    }
}

void FriendListWindow::HandleFriendRemoved(StringHash, VariantMap& eventData)
{
    using namespace Events::FriendAdded;
    const String& uuid = eventData[P_ACCOUNTUUID].GetString();
    Client::RelatedAccount::Releation rel = static_cast<Client::RelatedAccount::Releation>(eventData[P_RELATION].GetUInt());
    const String name = "ListViewItem_" + uuid;
    if (rel == Client::RelatedAccount::FriendRelationFriend)
    {
        Text* txt = dynamic_cast<Text*>(friendList_->GetChild(name, true));
        if (txt)
            txt->Remove();
    }
    else if (rel == Client::RelatedAccount::FriendRelationIgnore)
    {
        Text* txt = dynamic_cast<Text*>(ignoreList_->GetChild(name, true));
        if (txt)
            txt->Remove();
    }
}

void FriendListWindow::HandleFriendRemoveClicked(StringHash, VariantMap& eventData)
{
    friendPopup_->ShowPopup(false);
    using namespace MenuSelected;
    Menu* sender = dynamic_cast<Menu*>(eventData[P_ELEMENT].GetPtr());
    if (!sender)
        return;
    const String& uuid = friendPopup_->GetVar("AccountUuid").GetString();
    auto* client = GetSubsystem<FwClient>();
    client->RemoveFriend(uuid);
}

void FriendListWindow::HandleFriendWhisperClicked(StringHash, VariantMap& eventData)
{
    friendPopup_->ShowPopup(false);
    using namespace MenuSelected;
    Menu* sender = dynamic_cast<Menu*>(eventData[P_ELEMENT].GetPtr());
    if (!sender)
        return;
    const String& name = friendPopup_->GetVar("CharacterName").GetString();

    using namespace Events::WhisperTo;
    VariantMap& e = GetEventDataMap();
    e[P_NAME] = name;
    SendEvent(Events::E_WHISPERTO, e);
}

void FriendListWindow::HandleFriendItemClicked(StringHash, VariantMap& eventData)
{
    using namespace ClickEnd;
    MouseButton button = static_cast<MouseButton>(eventData[P_BUTTON].GetUInt());
    Text* elem = dynamic_cast<Text*>(eventData[P_ELEMENT].GetPtr());
    if (!elem)
        return;
    if (button == MOUSEB_RIGHT)
    {
        int x = eventData[P_X].GetInt();
        int y = eventData[P_Y].GetInt();
        friendPopup_->SetPosition(x, y);
        friendPopup_->ShowPopup(true);
        friendPopup_->SetVar("AccountUuid", elem->GetVar("AccountUuid").GetString());
        friendPopup_->SetVar("CharacterName", elem->GetVar("CharacterName").GetString());
    }
}

void FriendListWindow::HandleFriendAdded(StringHash, VariantMap& eventData)
{
    addFriendEdit_->SetText("");
    using namespace Events::FriendAdded;
    const String& uuid = eventData[P_ACCOUNTUUID].GetString();
    auto* client = GetSubsystem<FwClient>();
    client->GetPlayerInfoByAccount(std::string(uuid.CString()));
}

void FriendListWindow::UpdateItem(ListView* lv, const Client::RelatedAccount& f)
{
    const String name = "ListViewItem_" + String(f.accountUuid.c_str());
    Text* txt = dynamic_cast<Text*>(lv->GetChild(name, true));
    if (!txt)
    {
        txt = lv->CreateChild<Text>();
        txt->SetName(name);
        SubscribeToEvent(txt, E_CLICKEND, URHO3D_HANDLER(FriendListWindow, HandleFriendItemClicked));
    }
    String text(f.name.c_str());
    if (f.name.compare(f.currentName) != 0 && f.status == Client::RelatedAccount::OnlineStatusOnline)
        text.AppendWithFormat(" (%s)", f.currentName.c_str());

    txt->SetText(text);
    if (f.status != Client::RelatedAccount::OnlineStatusOnline)
        txt->SetStyle("FriendListItem");
    else
        txt->SetStyle("FriendListItemOnline");
    txt->SetVar("AccountUuid", String(f.accountUuid.c_str()));
    txt->SetVar("CharacterName", String(f.currentName.c_str()));
    txt->EnableLayoutUpdate();
    txt->UpdateLayout();
    lv->AddItem(txt);
    lv->EnableLayoutUpdate();
    lv->UpdateLayout();
}

void FriendListWindow::HandlePlayerLoggedOut(StringHash, VariantMap& eventData)
{
    using namespace Events::PlayerLoggedOut;
    const String& uuid = eventData[P_ACCOUNTUUID].GetString();
    auto* client = GetSubsystem<FwClient>();
    auto* acc = client->GetRelatedAccount(uuid);
    if (acc == nullptr)
        return;

    if (acc->relation == AB::Entities::FriendRelationFriend)
        UpdateItem(friendList_, *acc);
    else if (acc->relation == AB::Entities::FriendRelationIgnore)
        UpdateItem(ignoreList_, *acc);
}

void FriendListWindow::HandlePlayerLoggedIn(StringHash, VariantMap& eventData)
{
    using namespace Events::PlayerLoggedIn;
    const String& uuid = eventData[P_ACCOUNTUUID].GetString();
    auto* client = GetSubsystem<FwClient>();
    auto* acc = client->GetRelatedAccount(uuid);
    if (acc == nullptr)
        return;

    if (acc->relation == AB::Entities::FriendRelationFriend)
        UpdateItem(friendList_, *acc);
    else if (acc->relation == AB::Entities::FriendRelationIgnore)
        UpdateItem(ignoreList_, *acc);
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

void FriendListWindow::HandleGotPlayerInfo(StringHash, VariantMap& eventData)
{
    using namespace Events::GotPlayerInfo;
    const String& uuid = eventData[P_ACCOUNTUUID].GetString();
        auto* client = GetSubsystem<FwClient>();
    auto* acc = client->GetRelatedAccount(uuid);
    if (acc == nullptr)
        return;

    if (acc->relation == AB::Entities::FriendRelationFriend)
        UpdateItem(friendList_, *acc);
    else if (acc->relation == AB::Entities::FriendRelationIgnore)
        UpdateItem(ignoreList_, *acc);
}
