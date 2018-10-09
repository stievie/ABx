#pragma once

class FriendListWindow : public Window
{
    URHO3D_OBJECT(FriendListWindow, Window);
private:
    void SubscribeEvents();
public:
    static void RegisterObject(Context* context);

    FriendListWindow(Context* context);
    ~FriendListWindow();
};

