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

#include <AB/Dialogs.h>
#include "DialogWindow.h"

static const StringHash WINDOW_OPTIONS("OptionsWindow");
static const StringHash WINDOW_CHAT("CatWindow");
static const StringHash WINDOW_MAIL("MailWindow");
static const StringHash WINDOW_PARTY("PartyWindow");
static const StringHash WINDOW_PINGDOT("PingDot");
static const StringHash WINDOW_TARGET("TargetWindow");
static const StringHash WINDOW_NEWMAIL("NewMailWindow");
static const StringHash WINDOW_MISSIONMAP("MissionMapWindow");
static const StringHash WINDOW_SKILLBAR("SkillBarWindow");
static const StringHash WINDOW_FRIENDLIST("FriendListWindow");
static const StringHash WINDOW_GAMEMESSAGES("GameMessagesWindow");
static const StringHash WINDOW_EFFECTS("EffectsWindow");
static const StringHash WINDOW_INVENTORY("InventoryWindow");
static const StringHash WINDOW_GUILD("GuildWindow");
static const StringHash WINDOW_SKILLS("SkillsWindow");
static const StringHash WINDOW_EQUIPMENT("EquipmentWindow");
static const StringHash WINDOW_HEALTHBAR("HealthBarWindow");
static const StringHash WINDOW_ENERGYBAR("EnergyBarWindow");
static const StringHash WINDOW_DAMAGE("DamageWindow");

class WindowManager : public Object
{
    URHO3D_OBJECT(WindowManager, Object)
private:
    HashMap<StringHash, SharedPtr<UIElement>> windows_;
public:
    WindowManager(Context* context);
    ~WindowManager() override = default;

    const HashMap<StringHash, SharedPtr<UIElement>>& GetWindows() const
    {
        return windows_;
    }
    bool IsLoaded(const StringHash& hash) const
    {
        return windows_.Contains(hash);
    }

    SharedPtr<UIElement> GetWindow(const StringHash& hash, bool addToUi = false);
    SharedPtr<DialogWindow> GetDialog(AB::Dialogs dialog);

    void SaveWindows();
};

