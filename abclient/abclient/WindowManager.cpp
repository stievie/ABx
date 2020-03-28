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

#include "stdafx.h"
#include "WindowManager.h"
#include "OptionsWindow.h"
#include "ChatWindow.h"
#include "MailWindow.h"
#include "PartyWindow.h"
#include "PingDot.h"
#include "TargetWindow.h"
#include "GameMenu.h"
#include "MapWindow.h"
#include "Options.h"
#include "NewMailWindow.h"
#include "MissionMapWindow.h"
#include "SkillBarWindow.h"
#include "FriendListWindow.h"
#include "GameMessagesWindow.h"
#include "EffectsWindow.h"
#include "InventoryWindow.h"
#include "AccountChestDialog.h"
#include "GuildWindow.h"
#include "SkillsWindow.h"
#include "EquipmentWindow.h"
#include "ActorResourceBar.h"
#include "DamageWindow.h"

WindowManager::WindowManager(Context* context) :
    Object(context)
{
}

SharedPtr<UIElement> WindowManager::GetWindow(const StringHash& hash, bool addToUi /* = false */)
{
    Options* opts = GetSubsystem<Options>();
    if (!windows_.Contains(hash))
    {
        if (hash == WINDOW_OPTIONS)
        {
            SharedPtr<OptionsWindow> wnd = MakeShared<OptionsWindow>(context_);
            opts->LoadWindow(wnd);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_CHAT)
        {
            SharedPtr<ChatWindow> wnd = MakeShared<ChatWindow>(context_);
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_MAIL)
        {
            SharedPtr<MailWindow> wnd = MakeShared<MailWindow>(context_);
            opts->LoadWindow(wnd);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_NEWMAIL)
        {
            SharedPtr<NewMailWindow> wnd = MakeShared<NewMailWindow>(context_);
            opts->LoadWindow(wnd);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_INVENTORY)
        {
            SharedPtr<InventoryWindow> wnd = MakeShared<InventoryWindow>(context_);
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
            if (wnd->IsVisible())
                wnd->GetInventory();
        }
        else if (hash == WINDOW_EQUIPMENT)
        {
            SharedPtr<EquipmentWindow> wnd = MakeShared<EquipmentWindow>(context_);
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_SKILLS)
        {
            SharedPtr<SkillsWindow> wnd = MakeShared<SkillsWindow>(context_);
            opts->LoadWindow(wnd);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_PARTY)
        {
            SharedPtr<PartyWindow> wnd = MakeShared<PartyWindow>(context_);
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_FRIENDLIST)
        {
            SharedPtr<FriendListWindow> wnd = MakeShared<FriendListWindow>(context_);
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
            if (wnd->IsVisible())
                wnd->GetList();
        }
        else if (hash == WINDOW_GUILD)
        {
            SharedPtr<GuildWindow> wnd = MakeShared<GuildWindow>(context_);
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
            if (wnd->IsVisible())
                wnd->UpdateAll();
        }
        else if (hash == WINDOW_PINGDOT)
        {
            SharedPtr<PingDot> wnd = MakeShared<PingDot>(context_);
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_TARGET)
        {
            SharedPtr<TargetWindow> wnd = MakeShared<TargetWindow>(context_);
            wnd->SetVisible(false);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_GAMEMESSAGES)
        {
            SharedPtr<GameMessagesWindow> wnd = MakeShared<GameMessagesWindow>(context_);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_EFFECTS)
        {
            SharedPtr<EffectsWindow> wnd = MakeShared<EffectsWindow>(context_);
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_MISSIONMAP)
        {
            SharedPtr<MissionMapWindow> wnd = MakeShared<MissionMapWindow>(context_);
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_SKILLBAR)
        {
            SharedPtr<SkillBarWindow> wnd = MakeShared<SkillBarWindow>(context_);
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_HEALTHBAR)
        {
            SharedPtr<ActorHealthBar> wnd = MakeShared<ActorHealthBar>(context_);
            opts->LoadWindow(wnd);
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_ENERGYBAR)
        {
            SharedPtr<ActorEnergyBar> wnd = MakeShared<ActorEnergyBar>(context_);
            opts->LoadWindow(wnd);
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_DAMAGE)
        {
            SharedPtr<DamageWindow> wnd = MakeShared<DamageWindow>(context_);
            opts->LoadWindow(wnd);
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
    }

    if (windows_.Contains(hash))
    {
        auto wnd = windows_[hash];
        if (addToUi)
        {
            UIElement* root = GetSubsystem<UI>()->GetRoot();
            if (!root->GetChild(wnd->GetName()))
                root->AddChild(wnd);
        }
        return wnd;
    }
    return SharedPtr<UIElement>();
}

SharedPtr<DialogWindow> WindowManager::GetDialog(AB::Dialogs dialog)
{
    SharedPtr<DialogWindow> result;
    switch (dialog)
    {
    case AB::DialogUnknown:
        break;
    case AB::DialogAccountChest:
    {
        UIElement* root = GetSubsystem<UI>()->GetRoot();
        UIElement* wnd = root->GetChild(AccountChestDialog::GetTypeNameStatic());
        if (!wnd)
        {
            wnd = new AccountChestDialog(context_);
            root->AddChild(wnd);
        }
        result = dynamic_cast<DialogWindow*>(wnd);
        break;
    }
    case AB::DialogMerchantItems:
        break;
    case AB::DialogSmithItems:
        break;
    default:
        break;
    }
    return result;
}

void WindowManager::SaveWindows()
{
    Options* opts = GetSubsystem<Options>();
    for (const auto& wnd : windows_)
    {
        opts->SaveWindow(wnd.second_);
    }
}
