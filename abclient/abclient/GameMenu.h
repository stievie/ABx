#pragma once

class GameMenu : public UIElement
{
    URHO3D_OBJECT(GameMenu, UIElement);
private:
    SharedPtr<BorderImage> menuBar_;
    SharedPtr<Menu> menu_;
    SharedPtr<Menu> serversMenu_;
    void CreateMenuBar();
    Menu* CreateMenu(UIElement* parent, const String& title, const String& shortcut);
    Menu* CreateMenuItem(UIElement* parent, const String& title, const String& shortcut, EventHandler* handler);
    BorderImage* CreateSeparator(UIElement* parent);
    Window* CreatePopup(Menu* baseMenu);
    void HandleExitUsed(StringHash eventType, VariantMap& eventData);
    void HandleCreditsUsed(StringHash eventType, VariantMap& eventData);
    void HandleServerUsed(StringHash eventType, VariantMap& eventData);
    void HandleLogoutUsed(StringHash eventType, VariantMap& eventData);
    void HandleSelectCharUsed(StringHash eventType, VariantMap& eventData);
    void HandleOptionsUsed(StringHash eventType, VariantMap& eventData);
    void HandleMailUsed(StringHash eventType, VariantMap& eventData);
    void HandlePartyWindowUsed(StringHash eventType, VariantMap& eventData);
    void HandleInventoryWindowUsed(StringHash eventType, VariantMap& eventData);
    void HandleMapUsed(StringHash eventType, VariantMap& eventData);
    void HandleMissionMapUsed(StringHash eventType, VariantMap& eventData);
    void HandleGotServices(StringHash eventType, VariantMap& eventData);
    void HandleFriendsUsed(StringHash eventType, VariantMap& eventData);
    void HandleGuildWindowUsed(StringHash eventType, VariantMap& eventData);
    void UpdateServers();
public:
    static void RegisterObject(Context* context);

    GameMenu(Context* context);
    ~GameMenu();
};

