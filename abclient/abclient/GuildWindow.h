#pragma once

#include "TabGroup.h"

class GuildWindow : public Window
{
    URHO3D_OBJECT(GuildWindow, Window);
private:
    SharedPtr<TabGroup> tabgroup_;
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleTabSelected(StringHash eventType, VariantMap& eventData);
    void LoadWindow(Window* wnd, const String& fileName);
    TabElement* CreateTab(TabGroup* tabs, const String& page);
    void CreatePageMembers(TabElement* tabElement);
    void CreatePageState(TabElement* tabElement);
public:
    static void RegisterObject(Context* context);

    GuildWindow(Context* context);
    ~GuildWindow() override;

    void UpdateAll();
};

