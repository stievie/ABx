#pragma once

class FriendListWindow : public Window
{
    URHO3D_OBJECT(FriendListWindow, Window);
private:
    SharedPtr<UIElement> friendContainer_;
    SharedPtr<UIElement> ignoreContainer_;

    void SubscribeEvents();
    void HandleGotFriendList(StringHash eventType, VariantMap& eventData);
    void HandlePlayerLoggedOut(StringHash eventType, VariantMap& eventData);
    void HandlePlayerLoggedIn(StringHash eventType, VariantMap& eventData);
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    FriendListWindow(Context* context);
    ~FriendListWindow();
};

