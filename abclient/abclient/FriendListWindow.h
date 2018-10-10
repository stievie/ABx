#pragma once

class FriendListWindow : public Window
{
    URHO3D_OBJECT(FriendListWindow, Window);
private:
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    FriendListWindow(Context* context);
    ~FriendListWindow();
};

