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
#include "EquipmentWindow.h"

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
            SharedPtr<OptionsWindow> wnd = SharedPtr<OptionsWindow>(new OptionsWindow(context_));
            opts->LoadWindow(wnd);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_CHAT)
        {
            SharedPtr<ChatWindow> wnd = SharedPtr<ChatWindow>(new ChatWindow(context_));
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_MAIL)
        {
            SharedPtr<MailWindow> wnd = SharedPtr<MailWindow>(new MailWindow(context_));
            opts->LoadWindow(wnd);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_NEWMAIL)
        {
            SharedPtr<NewMailWindow> wnd = SharedPtr<NewMailWindow>(new NewMailWindow(context_));
            opts->LoadWindow(wnd);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_INVENTORY)
        {
            SharedPtr<InventoryWindow> wnd = SharedPtr<InventoryWindow>(new InventoryWindow(context_));
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
            if (wnd->IsVisible())
                wnd->GetInventory();
        }
        else if (hash == WINDOW_EQUIPMENT)
        {
            SharedPtr<EquipmentWindow> wnd = SharedPtr<EquipmentWindow>(new EquipmentWindow(context_));
            opts->LoadWindow(wnd);
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_PARTY)
        {
            SharedPtr<PartyWindow> wnd = SharedPtr<PartyWindow>(new PartyWindow(context_));
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_FRIENDLIST)
        {
            SharedPtr<FriendListWindow> wnd = SharedPtr<FriendListWindow>(new FriendListWindow(context_));
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
            if (wnd->IsVisible())
                wnd->GetList();
        }
        else if (hash == WINDOW_GUILD)
        {
            SharedPtr<GuildWindow> wnd = SharedPtr<GuildWindow>(new GuildWindow(context_));
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
            if (wnd->IsVisible())
                wnd->UpdateAll();
        }
        else if (hash == WINDOW_PINGDOT)
        {
            SharedPtr<PingDot> wnd = SharedPtr<PingDot>(new PingDot(context_));
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_TARGET)
        {
            SharedPtr<TargetWindow> wnd = SharedPtr<TargetWindow>(new TargetWindow(context_));
            wnd->SetVisible(false);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_GAMEMESSAGES)
        {
            SharedPtr<GameMessagesWindow> wnd = SharedPtr<GameMessagesWindow>(new GameMessagesWindow(context_));
            wnd->SetVisible(false);
//            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_EFFECTS)
        {
            SharedPtr<EffectsWindow> wnd = SharedPtr<EffectsWindow>(new EffectsWindow(context_));
            wnd->SetVisible(true);
            //            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_MISSIONMAP)
        {
            SharedPtr<MissionMapWindow> wnd = SharedPtr<MissionMapWindow>(new MissionMapWindow(context_));
            wnd->SetVisible(true);
            opts->LoadWindow(wnd);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_SKILLBAR)
        {
            SharedPtr<SkillBarWindow> wnd = SharedPtr<SkillBarWindow>(new SkillBarWindow(context_));
            wnd->SetVisible(true);
//            opts->LoadWindow(wnd);
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
