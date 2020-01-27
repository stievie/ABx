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

#include <Urho3DAll.h>

class GameMenu : public UIElement
{
    URHO3D_OBJECT(GameMenu, UIElement)
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
    void HandleSkillsWindowUsed(StringHash eventType, VariantMap& eventData);
    void HandleEquipWindowUsed(StringHash eventType, VariantMap& eventData);
    void HandleMapUsed(StringHash eventType, VariantMap& eventData);
    void HandleMissionMapUsed(StringHash eventType, VariantMap& eventData);
    void HandleGotServices(StringHash eventType, VariantMap& eventData);
    void HandleFriendsUsed(StringHash eventType, VariantMap& eventData);
    void HandleGuildWindowUsed(StringHash eventType, VariantMap& eventData);
    void UpdateServers();
public:
    static void RegisterObject(Context* context);

    GameMenu(Context* context);
    ~GameMenu() override;
};

