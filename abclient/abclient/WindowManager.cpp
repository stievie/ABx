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

WindowManager::WindowManager(Context* context) :
    Object(context)
{
}

WindowManager::~WindowManager()
{
}

SharedPtr<UIElement> WindowManager::GetWindow(const StringHash& hash)
{
    if (!windows_.Contains(hash))
    {
        if (hash == WINDOW_OPTIONS)
        {
            SharedPtr<OptionsWindow> wnd = SharedPtr<OptionsWindow>(new OptionsWindow(context_));
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_CHAT)
        {
            SharedPtr<ChatWindow> wnd = SharedPtr<ChatWindow>(new ChatWindow(context_));
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_MAIL)
        {
            SharedPtr<MailWindow> wnd = SharedPtr<MailWindow>(new MailWindow(context_));
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_PARTY)
        {
            SharedPtr<PartyWindow> wnd = SharedPtr<PartyWindow>(new PartyWindow(context_));
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_PINGDOT)
        {
            SharedPtr<PingDot> wnd = SharedPtr<PingDot>(new PingDot(context_));
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_TARGET)
        {
            SharedPtr<TargetWindow> wnd = SharedPtr<TargetWindow>(new TargetWindow(context_));
            wnd->SetVisible(false);
            windows_[hash] = wnd;
        }
        else if (hash == WINDOW_GAMEMENU)
        {
            SharedPtr<GameMenu> wnd = SharedPtr<GameMenu>(new GameMenu(context_));
            wnd->SetVisible(true);
            windows_[hash] = wnd;
        }
    }

    if (windows_.Contains(hash))
        return windows_[hash];
    return SharedPtr<UIElement>();
}
