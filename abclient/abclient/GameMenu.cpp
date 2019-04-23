#include "stdafx.h"
#include "GameMenu.h"
#include "AbEvents.h"
#include "FwClient.h"
#include "Shortcuts.h"

#include <Urho3D/DebugNew.h>

void GameMenu::RegisterObject(Context* context)
{
    context->RegisterFactory<GameMenu>();
}

GameMenu::GameMenu(Context* context) :
    UIElement(context)
{
    SetName("GameMenu");
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    CreateMenuBar();
    SubscribeToEvent(AbEvents::E_GOTSERVICES, URHO3D_HANDLER(GameMenu, HandleGotServices));
    SetAlignment(HA_LEFT, VA_TOP);
}

GameMenu::~GameMenu()
{
    UnsubscribeFromAllEvents();
}

void GameMenu::CreateMenuBar()
{
    menuBar_ = CreateChild<BorderImage>("MenuBar");
    menuBar_->SetEnabled(true);
    menuBar_->SetLayout(LM_HORIZONTAL);
    menuBar_->SetStyle("EditorMenuBar");

    menu_ = CreateMenu(menuBar_, "Menu", String::EMPTY);
    menuBar_->SetHeight(20);
    menuBar_->SetFixedWidth(menu_->GetWidth());

    Shortcuts* scs = GetSubsystem<Shortcuts>();

    Window* popup = dynamic_cast<Window*>(menu_->GetPopup());
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_EXITPROGRAM, "Exit"),
        scs->GetShortcutName(AbEvents::E_SC_EXITPROGRAM),
        URHO3D_HANDLER(GameMenu, HandleExitUsed));
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_SHOWCREDITS, "Credits"),
        scs->GetShortcutName(AbEvents::E_SC_SHOWCREDITS),
        URHO3D_HANDLER(GameMenu, HandleCreditsUsed));
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_LOGOUT, "Logout"),
        scs->GetShortcutName(AbEvents::E_SC_LOGOUT),
        URHO3D_HANDLER(GameMenu, HandleLogoutUsed));
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_SELECTCHARACTER, "Select character"),
        scs->GetShortcutName(AbEvents::E_SC_SELECTCHARACTER),
        URHO3D_HANDLER(GameMenu, HandleSelectCharUsed));
    serversMenu_ = CreateMenu(popup, "Server", ">");
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_TOGGLEOPTIONS, "Options"),
        scs->GetShortcutName(AbEvents::E_SC_TOGGLEOPTIONS),
        URHO3D_HANDLER(GameMenu, HandleOptionsUsed));
    CreateSeparator(popup);

    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_TOGGLEMAILWINDOW, "Mail"),
        scs->GetShortcutName(AbEvents::E_SC_TOGGLEMAILWINDOW),
        URHO3D_HANDLER(GameMenu, HandleMailUsed));
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_TOGGLEFRIENDLISTWINDOW, "Friends"),
        scs->GetShortcutName(AbEvents::E_SC_TOGGLEFRIENDLISTWINDOW),
        URHO3D_HANDLER(GameMenu, HandleFriendsUsed));
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_TOGGLEPARTYWINDOW, "Party"),
        scs->GetShortcutName(AbEvents::E_SC_TOGGLEPARTYWINDOW),
        URHO3D_HANDLER(GameMenu, HandlePartyWindowUsed));
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_TOGGLEMISSIONMAPWINDOW, "Mission Map"),
        scs->GetShortcutName(AbEvents::E_SC_TOGGLEMISSIONMAPWINDOW),
        URHO3D_HANDLER(GameMenu, HandleMissionMapUsed));
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_TOGGLEMAP, "Map"),
        scs->GetShortcutName(AbEvents::E_SC_TOGGLEMAP),
        URHO3D_HANDLER(GameMenu, HandleMapUsed));
    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_TOGGLEINVENTORYWINDOW, "Inventory"),
        scs->GetShortcutName(AbEvents::E_SC_TOGGLEINVENTORYWINDOW),
        URHO3D_HANDLER(GameMenu, HandleInventoryWindowUsed));

    popup->SetWidth(40);
}

Menu* GameMenu::CreateMenu(UIElement* parent, const String& title, const String& shortcut)
{
    Menu* result = CreateMenuItem(parent, title, shortcut, nullptr);
    result->SetMinWidth(parent->GetWidth());
    CreatePopup(result);

    return result;
}

Menu* GameMenu::CreateMenuItem(UIElement* parent, const String& title, const String& shortcut, EventHandler* handler)
{
    Menu* menu = parent->CreateChild<Menu>();
    menu->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    menu->SetStyleAuto();
    menu->SetName(title);
    menu->SetMinWidth(parent->GetWidth());
    menu->SetWidth(parent->GetWidth());
    menu->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
    if (handler)
        SubscribeToEvent(menu, E_MENUSELECTED, handler);
    Text* menuText = menu->CreateChild<Text>();
    menuText->SetText(title);
    menuText->SetStyle("EditorMenuText");

    if (!shortcut.Empty())
    {
        Text* shortcutText = menu->CreateChild<Text>();
        shortcutText->SetText(shortcut);
        shortcutText->SetHorizontalAlignment(HA_RIGHT);
        shortcutText->SetTextAlignment(HA_RIGHT);
        shortcutText->SetStyle("EditorMenuText");
    }

    return menu;
}

BorderImage* GameMenu::CreateSeparator(UIElement* parent)
{
    BorderImage* sep = parent->CreateChild<BorderImage>();
    sep->SetStyle("EditorDivider");
    return sep;
}

Window* GameMenu::CreatePopup(Menu* baseMenu)
{
    Window* popup = baseMenu->CreateChild<Window>();
    popup->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    popup->SetStyle("MenuPopupWindow");
    popup->SetLayout(LM_VERTICAL, 1);
    baseMenu->SetPopup(popup);
    baseMenu->SetPopupOffset(IntVector2(0, baseMenu->GetHeight()));
    return popup;
}

void GameMenu::HandleExitUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_EXITPROGRAM, e);
}

void GameMenu::HandleCreditsUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_SHOWCREDITS, e);
}

void GameMenu::HandleServerUsed(StringHash, VariantMap& eventData)
{
    menu_->ShowPopup(false);
    using namespace MenuSelected;
    Menu* sender = static_cast<Menu*>(eventData[P_ELEMENT].GetPtr());
    FwClient* client = GetSubsystem<FwClient>();
    const String& id = sender->GetVar("Server ID").GetString();
    if (!id.Empty() && id.Compare(client->GetCurrentServerId()) != 0)
    {
        client->ChangeServer(id);
        UpdateServers();
    }
}

void GameMenu::HandleLogoutUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_LOGOUT, e);
}

void GameMenu::HandleSelectCharUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_SELECTCHARACTER, e);
}

void GameMenu::HandleOptionsUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_TOGGLEOPTIONS, e);
}

void GameMenu::HandleMailUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_TOGGLEMAILWINDOW, e);
}

void GameMenu::HandlePartyWindowUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_TOGGLEPARTYWINDOW , e);
}

void GameMenu::HandleInventoryWindowUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_TOGGLEINVENTORYWINDOW, e);
}

void GameMenu::HandleMapUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_TOGGLEMAP, e);
}

void GameMenu::HandleMissionMapUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_TOGGLEMISSIONMAPWINDOW, e);
}

void GameMenu::HandleGotServices(StringHash, VariantMap&)
{
    UpdateServers();
}

void GameMenu::HandleFriendsUsed(StringHash, VariantMap&)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_TOGGLEFRIENDLISTWINDOW, e);
}

void GameMenu::UpdateServers()
{
    FwClient* client = GetSubsystem<FwClient>();
    Window* popup = dynamic_cast<Window*>(serversMenu_->GetPopup());
    popup->RemoveAllChildren();
    popup->SetLayout(LM_VERTICAL, 1);
    serversMenu_->SetPopupOffset(IntVector2(menu_->GetPopup()->GetWidth(), 0));
    String cId = client->GetCurrentServerId();
    const std::map<std::string, AB::Entities::Service>& servs = client->GetServices();
    for (const auto& serv : servs)
    {
        String sId = String(serv.first.c_str());
        Menu* mnu = CreateMenuItem(popup, String(serv.second.name.c_str()), String::EMPTY,
            URHO3D_HANDLER(GameMenu, HandleServerUsed));
        mnu->SetSelected(sId.Compare(cId) == 0);
        mnu->SetVar("Server ID", sId);
    }
}
