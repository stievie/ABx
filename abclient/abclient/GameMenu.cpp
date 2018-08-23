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
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    CreateMenuBar();
    SubscribeToEvent(AbEvents::E_GOTSERVICES, URHO3D_HANDLER(GameMenu, HandleGotServices));
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

    menu_ = CreateMenu(menuBar_, "Menu");
    menuBar_->SetHeight(20);
    menuBar_->SetFixedWidth(menu_->GetWidth());
    SubscribeToEvent(menu_, E_MENUSELECTED, URHO3D_HANDLER(GameMenu, HandleRootMenuUsed));

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    String caption;

    Window* popup = dynamic_cast<Window*>(menu_->GetPopup());
    CreateMenuItem(popup, "Exit", URHO3D_HANDLER(GameMenu, HandleExitUsed));
    CreateMenuItem(popup, "Logout", URHO3D_HANDLER(GameMenu, HandleLogoutUsed));
    CreateMenuItem(popup, "Select Character", URHO3D_HANDLER(GameMenu, HandleSelectCharUsed));
    serversMenu_ = CreateMenu(popup, "Server >");
    CreateSeparator(popup);
    CreateMenuItem(popup, "Options", URHO3D_HANDLER(GameMenu, HandleOptionsUsed));
    CreateSeparator(popup);
    CreateMenuItem(popup, "Mail", URHO3D_HANDLER(GameMenu, HandleMailUsed));

    CreateMenuItem(popup, scs->GetCaption(AbEvents::E_SC_TOGGLEPARTYWINDOW, "Party"),
        URHO3D_HANDLER(GameMenu, HandlePartyWindowUsed));

    popup->SetWidth(40);

}

Menu* GameMenu::CreateMenu(UIElement* parent, const String& title)
{
    Menu* result = CreateMenuItem(parent, title, nullptr);
    auto menuText = result->GetChildren()[0];
    result->SetMaxWidth(menuText->GetWidth() + 20);
    CreatePopup(result);

    return result;
}

Menu* GameMenu::CreateMenuItem(UIElement* parent, const String& title, EventHandler* handler)
{
    Menu* menu = parent->CreateChild<Menu>();
    menu->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    menu->SetStyleAuto();
    menu->SetName(title);
    menu->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
    if (handler)
        SubscribeToEvent(menu, E_MENUSELECTED, handler);
    Text* menuText = menu->CreateChild<Text>();
    menuText->SetText(title);
    menuText->SetStyle("EditorMenuText");

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

void GameMenu::HandleRootMenuUsed(StringHash eventType, VariantMap& eventData)
{
}

void GameMenu::HandleExitUsed(StringHash eventType, VariantMap& eventData)
{
    Engine* engine = context_->GetSubsystem<Engine>();
    engine->Exit();
}

void GameMenu::HandleServerUsed(StringHash eventType, VariantMap& eventData)
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

void GameMenu::HandleLogoutUsed(StringHash eventType, VariantMap& eventData)
{
    menu_->ShowPopup(false);
    SetVisible(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(E_GAMEMENU_LOGOUT, e);
}

void GameMenu::HandleSelectCharUsed(StringHash eventType, VariantMap& eventData)
{
    menu_->ShowPopup(false);
    SetVisible(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(E_GAMEMENU_SELECTCHAR, e);
}

void GameMenu::HandleOptionsUsed(StringHash eventType, VariantMap& eventData)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(E_GAMEMENU_OPTIONSWINDOW, e);
}

void GameMenu::HandleMailUsed(StringHash eventType, VariantMap& eventData)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(E_GAMEMENU_MAIL, e);
}

void GameMenu::HandlePartyWindowUsed(StringHash eventType, VariantMap& eventData)
{
    menu_->ShowPopup(false);
    VariantMap& e = GetEventDataMap();
    SendEvent(E_GAMEMENU_PARTYWINDOW , e);
}

void GameMenu::HandleGotServices(StringHash eventType, VariantMap & eventData)
{
    UpdateServers();
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
        Menu* mnu = CreateMenuItem(popup, String(serv.second.name.c_str()), URHO3D_HANDLER(GameMenu, HandleServerUsed));
        mnu->SetSelected(sId.Compare(cId) == 0);
        mnu->SetVar("Server ID", sId);
    }
}
