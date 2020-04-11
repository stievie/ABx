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
#include "FriendListWindow.h"
#include "Shortcuts.h"
#include "FwClient.h"
#include "Player.h"

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
    Text* caption = GetChildStaticCast<Text>("Caption", true);
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLEFRIENDLISTWINDOW, "Friends", true));

    friendList_ = GetChildStaticCast<ListView>("FriendListView", true);
    ignoreList_ = GetChildStaticCast<ListView>("IgnoreListView", true);
    addFriendEdit_ = GetChildStaticCast<LineEdit>("AddFriendEdit", true);
    addIgnoreEdit_ = GetChildStaticCast<LineEdit>("AddIgnoredEdit", true);
    statusDropdown_ = GetChildStaticCast<DropDownList>("StatusDropdown", true);

    auto createStatusDropdownItem = [this](const String& text, AB::Packets::Server::PlayerInfo::Status status) -> Text*
    {
        Text* result = new Text(context_);
        result->SetText(text);
        result->SetVar("Value", static_cast<int>(status));
        result->SetStyle("DropDownItemEnumText");
        return result;
    };
    statusDropdown_->AddItem(createStatusDropdownItem("Online", AB::Packets::Server::PlayerInfo::Status::OnlineStatusOnline));
    statusDropdown_->AddItem(createStatusDropdownItem("Away", AB::Packets::Server::PlayerInfo::Status::OnlineStatusAway));
    statusDropdown_->AddItem(createStatusDropdownItem("Do not disturb", AB::Packets::Server::PlayerInfo::Status::OnlineStatusDoNotDisturb));
    statusDropdown_->AddItem(createStatusDropdownItem("Offline", AB::Packets::Server::PlayerInfo::Status::OnlineStatusInvisible));

    SetSize(272, 128);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(5, graphics->GetHeight() / 2 + GetHeight());
    SetVisible(true);

    SetStyleAuto();

    CreateMenus();

    SubscribeEvents();
}

void FriendListWindow::CreateMenus()
{
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
        menuText->SetText("Send Mail");
        menuText->SetStyle("EditorMenuText");
        item->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
        item->SetMinSize(menuText->GetSize() + IntVector2(4, 4));
        item->SetSize(item->GetMinSize());
        if (item->GetWidth() > width)
            width = item->GetWidth();
        if (item->GetHeight() > height)
            height = item->GetHeight();
        SubscribeToEvent(item, E_MENUSELECTED, URHO3D_HANDLER(FriendListWindow, HandleFriendSendMailClicked));
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
        initialized_ = true;
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
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(FriendListWindow, HandleCloseClicked));
    Button* addFriendButton = GetChildStaticCast<Button>("AddFriendButton", true);
    SubscribeToEvent(addFriendButton, E_RELEASED, URHO3D_HANDLER(FriendListWindow, HandleAddFriendClicked));
    Button* addIgnoreButton = GetChildStaticCast<Button>("AddIgnoredButton", true);
    SubscribeToEvent(addIgnoreButton, E_RELEASED, URHO3D_HANDLER(FriendListWindow, HandleAddIgnoreClicked));
    SubscribeToEvent(statusDropdown_, E_ITEMSELECTED, URHO3D_HANDLER(FriendListWindow, HandleStatusDropdownSelected));

    SubscribeToEvent(addFriendEdit_, E_TEXTFINISHED, URHO3D_HANDLER(FriendListWindow, HandleAddFriendClicked));
    SubscribeToEvent(addIgnoreEdit_, E_TEXTFINISHED, URHO3D_HANDLER(FriendListWindow, HandleAddIgnoreClicked));

    SubscribeToEvent(Events::E_GOT_PLAYERINFO, URHO3D_HANDLER(FriendListWindow, HandleGotPlayerInfo));
    SubscribeToEvent(Events::E_GOT_FRIENDLIST, URHO3D_HANDLER(FriendListWindow, HandleGotFriendList));
    SubscribeToEvent(Events::E_FRIENDADDED, URHO3D_HANDLER(FriendListWindow, HandleFriendAdded));
    SubscribeToEvent(Events::E_FRIENDREMOVED, URHO3D_HANDLER(FriendListWindow, HandleFriendRemoved));
}

void FriendListWindow::HandleStatusDropdownSelected(StringHash, VariantMap& eventData)
{
    using namespace ItemSelected;
    unsigned sel = eventData[P_SELECTION].GetUInt();
    DropDownList* list = dynamic_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
    UIElement* elem = list->GetItem(sel);
    if (elem)
    {
        AB::Packets::Server::PlayerInfo::Status status = static_cast<AB::Packets::Server::PlayerInfo::Status>(elem->GetVar("Value").GetUInt());
        auto* client = GetSubsystem<FwClient>();
        client->SetOnlineStatus(status);
    }
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
    client->GetPlayerInfoByAccount(client->GetAccountUuid(), AB::GameProtocol::PlayerInfoFieldOnlineStatus);
    for (const auto& f : fl)
    {
        // Should trigger GotPlayerInfo where we add the item
        client->GetPlayerInfoByAccount(f, AB::GameProtocol::PlayerInfoFieldsAll);
    }
}

void FriendListWindow::UpdateSelf(const AB::Packets::Server::PlayerInfo& acc)
{
    if (acc.fields & AB::GameProtocol::PlayerInfoFieldOnlineStatus)
    {
        for (unsigned i = 0; i < statusDropdown_->GetNumItems(); ++i)
        {
            auto* item = statusDropdown_->GetItem(i);
            if (item->GetVar("Value").GetUInt() == static_cast<unsigned>(acc.status))
            {
                statusDropdown_->SetSelection(i);
                break;
            }
        }
    }
}

void FriendListWindow::AddFriend()
{
    const String& name = addFriendEdit_->GetText();
    if (name.Empty())
        return;
    auto* client = GetSubsystem<FwClient>();
    client->AddFriend(name, AB::Entities::FriendRelationFriend);
}

void FriendListWindow::AddIgnore()
{
    const String& name = addIgnoreEdit_->GetText();
    if (name.Empty())
        return;
    auto* client = GetSubsystem<FwClient>();
    client->AddFriend(name, AB::Entities::FriendRelationIgnore);
}

void FriendListWindow::HandleFriendRemoved(StringHash, VariantMap& eventData)
{
    using namespace Events::FriendAdded;
    const String& uuid = eventData[P_ACCOUNTUUID].GetString();
    AB::Packets::Server::PlayerInfo::Relation rel = static_cast<AB::Packets::Server::PlayerInfo::Relation>(eventData[P_RELATION].GetUInt());
    const String name = "ListViewItem_" + uuid;
    if (rel == AB::Packets::Server::PlayerInfo::FriendRelationFriend)
    {
        Text* txt = friendList_->GetChildStaticCast<Text>(name, true);
        if (txt)
            txt->Remove();
    }
    else if (rel == AB::Packets::Server::PlayerInfo::FriendRelationIgnore)
    {
        Text* txt = ignoreList_->GetChildStaticCast<Text>(name, true);
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

void FriendListWindow::HandleFriendSendMailClicked(StringHash, VariantMap& eventData)
{
    friendPopup_->ShowPopup(false);
    using namespace MenuSelected;
    Menu* sender = dynamic_cast<Menu*>(eventData[P_ELEMENT].GetPtr());
    if (!sender)
        return;
    const String& name = friendPopup_->GetVar("CharacterName").GetString();

    using namespace Events::SendMailTo;
    VariantMap& e = GetEventDataMap();
    e[P_NAME] = name;
    SendEvent(Events::E_SENDMAILTO, e);
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
        friendPopup_->SetVar("AccountUuid", elem->GetVar("AccountUuid").GetString());
        friendPopup_->SetVar("CharacterName", elem->GetVar("CharacterName").GetString());
        friendPopup_->ShowPopup(true);
    }
}

void FriendListWindow::HandleFriendAdded(StringHash, VariantMap& eventData)
{
    addFriendEdit_->SetText("");
    using namespace Events::FriendAdded;
    const String& uuid = eventData[P_ACCOUNTUUID].GetString();
    auto* client = GetSubsystem<FwClient>();
    client->GetPlayerInfoByAccount(std::string(uuid.CString()), AB::GameProtocol::PlayerInfoFieldsAll);
}

void FriendListWindow::UpdateItem(ListView* lv, const AB::Packets::Server::PlayerInfo& f)
{
    const String name = "ListViewItem_" + String(f.accountUuid.c_str());
    Text* txt = lv->GetChildStaticCast<Text>(name, true);
    if (!txt)
    {
        txt = lv->CreateChild<Text>();
        txt->SetName(name);
        SubscribeToEvent(txt, E_CLICKEND, URHO3D_HANDLER(FriendListWindow, HandleFriendItemClicked));
    }
    String text(f.nickName.c_str());
    if (f.nickName.compare(f.currentName) != 0 && !Client::IsOffline(f.status))
        text.AppendWithFormat(" (%s)", f.currentName.c_str());

    auto* client = GetSubsystem<FwClient>();
    if (!Client::IsOffline(f.status))
    {
        const AB::Entities::Game* game = client->GetGame(String(f.currentMap.c_str()));
        if (game)
            text.AppendWithFormat(" [%s]", game->name.c_str());
    }

    switch (f.status)
    {
    case AB::Packets::Server::PlayerInfo::OnlineStatusAway:
        text.Append(" (AFK)");
        break;
    case AB::Packets::Server::PlayerInfo::OnlineStatusDoNotDisturb:
        text.Append(" (DND)");
        break;
    default:
        break;
    }

    txt->SetText(text);
    if (Client::IsOffline(f.status))
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

void FriendListWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void FriendListWindow::HandleAddFriendClicked(StringHash, VariantMap&)
{
    AddFriend();
}

void FriendListWindow::HandleAddIgnoreClicked(StringHash, VariantMap&)
{
    AddIgnore();
}

void FriendListWindow::HandleGotPlayerInfo(StringHash, VariantMap& eventData)
{
    using namespace Events::GotPlayerInfo;
    const String& uuid = eventData[P_ACCOUNTUUID].GetString();
        auto* client = GetSubsystem<FwClient>();
    auto* acc = client->GetRelatedAccount(uuid);
    if (acc == nullptr)
        return;

    if (acc->accountUuid.compare(client->GetAccountUuid()) == 0)
        UpdateSelf(*acc);
    else if (acc->relation == AB::Packets::Server::PlayerInfo::FriendRelationFriend)
        UpdateItem(friendList_, *acc);
    else if (acc->relation == AB::Packets::Server::PlayerInfo::FriendRelationIgnore)
        UpdateItem(ignoreList_, *acc);
}
