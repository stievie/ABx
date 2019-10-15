#pragma once

#include "Receiver.h"

class Player;

class FriendListWindow : public Window
{
    URHO3D_OBJECT(FriendListWindow, Window);
private:
    SharedPtr<ListView> friendList_;
    SharedPtr<ListView> ignoreList_;
    SharedPtr<LineEdit> addFriendEdit_;
    SharedPtr<LineEdit> addIgnoreEdit_;
    SharedPtr<Menu> friendPopup_;
    SharedPtr<Menu> ignorePopup_;
    SharedPtr<DropDownList> statusDropdown_;
    bool initialized_{ false };

    void CreateMenus();
    void SubscribeEvents();
    void HandleStatusDropdownSelected(StringHash eventType, VariantMap& eventData);
    void HandleGotFriendList(StringHash eventType, VariantMap& eventData);
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleAddFriendClicked(StringHash eventType, VariantMap& eventData);
    void HandleAddIgnoreClicked(StringHash eventType, VariantMap& eventData);
    void HandleGotPlayerInfo(StringHash eventType, VariantMap& eventData);
    void HandleFriendAdded(StringHash eventType, VariantMap& eventData);
    void HandleFriendRemoved(StringHash eventType, VariantMap& eventData);
    void HandleFriendRemoveClicked(StringHash eventType, VariantMap& eventData);
    void HandleFriendWhisperClicked(StringHash eventType, VariantMap& eventData);
    void HandleFriendSendMailClicked(StringHash eventType, VariantMap& eventData);
    void HandleFriendItemClicked(StringHash eventType, VariantMap& eventData);
    void UpdateItem(ListView* lv, const Client::RelatedAccount& f);
    void UpdateAll();
    void UpdateSelf(const Client::RelatedAccount& acc);
public:
    static void RegisterObject(Context* context);

    FriendListWindow(Context* context);
    ~FriendListWindow();

    void GetList();
    void Clear();
};

