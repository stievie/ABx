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

#pragma once

#include "Receiver.h"
#include <AB/Packets/ServerPackets.h>
#include <Urho3DAll.h>

class Player;

class FriendListWindow : public Window
{
    URHO3D_OBJECT(FriendListWindow, Window)
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
    void UpdateItem(ListView* lv, const AB::Packets::Server::PlayerInfo& f);
    void UpdateAll();
    void UpdateSelf(const AB::Packets::Server::PlayerInfo& acc);
    void AddFriend();
    void AddIgnore();
public:
    static void RegisterObject(Context* context);

    FriendListWindow(Context* context);
    ~FriendListWindow() override;

    void GetList();
    void Clear();
};

