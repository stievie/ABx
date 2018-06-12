#pragma once

static const StringHash E_GAMEMENU_LOGOUT = StringHash("GameMemu Logout");
static const StringHash E_GAMEMENU_SELECTCHAR = StringHash("GameMemu SelectChar");
static const StringHash E_GAMEMENU_MAIL = StringHash("GameMemu Mail");

class GameMenu : public UIElement
{
    URHO3D_OBJECT(GameMenu, UIElement);
private:
    SharedPtr<BorderImage> menuBar_;
    SharedPtr<Menu> menu_;
    SharedPtr<Menu> serversMenu_;
    void CreateMenuBar();
    Menu* CreateMenu(UIElement* parent, const String& title);
    Menu* CreateMenuItem(UIElement* parent, const String& title, EventHandler* handler);
    BorderImage* CreateSeparator(UIElement* parent);
    Window* CreatePopup(Menu* baseMenu);
    void HandleRootMenuUsed(StringHash eventType, VariantMap& eventData);
    void HandleExitUsed(StringHash eventType, VariantMap& eventData);
    void HandleServerUsed(StringHash eventType, VariantMap& eventData);
    void HandleLogoutUsed(StringHash eventType, VariantMap& eventData);
    void HandleSelectCharUsed(StringHash eventType, VariantMap& eventData);
    void HandleOptionsUsed(StringHash eventType, VariantMap& eventData);
    void HandleMailUsed(StringHash eventType, VariantMap& eventData);
    void UpdateServers();
public:
    static void RegisterObject(Context* context);

    GameMenu(Context* context);
    ~GameMenu();
};

