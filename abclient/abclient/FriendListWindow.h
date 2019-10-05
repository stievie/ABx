#pragma once

#include "Receiver.h"

class FriendListWindow : public Window
{
    URHO3D_OBJECT(FriendListWindow, Window);
private:
    SharedPtr<ListView> friendList_;
    SharedPtr<ListView> ignoreList_;
    SharedPtr<LineEdit> addFriendEdit_;
    SharedPtr<LineEdit> addIgnoreEdit_;
    bool initialized_{ false };

    void SubscribeEvents();
    void HandleGotFriendList(StringHash eventType, VariantMap& eventData);
    void HandlePlayerLoggedOut(StringHash eventType, VariantMap& eventData);
    void HandlePlayerLoggedIn(StringHash eventType, VariantMap& eventData);
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleAddFriendClicked(StringHash eventType, VariantMap& eventData);
    void HandleAddIgnoreClicked(StringHash eventType, VariantMap& eventData);
    void AddItem(ListView* lv, const Client::RelatedAccount& f);
    void UpdateAll();
public:
    static void RegisterObject(Context* context);

    FriendListWindow(Context* context);
    ~FriendListWindow();

    void GetList();
    void Clear();
};

