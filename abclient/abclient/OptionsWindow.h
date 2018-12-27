#pragma once

#include "TabGroup.h"
#include "Shortcuts.h"

class OptionsWindow : public Window
{
    URHO3D_OBJECT(OptionsWindow, Window);
private:
    SharedPtr<TabGroup> tabgroup_;
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleTabSelected(StringHash eventType, VariantMap& eventData);
    void HandleFovSliderChanged(StringHash eventType, VariantMap& eventData);
    void SubscribeEvents();
    TabElement* CreateTab(TabGroup* tabs, const String& page);
    void CreatePageGeneral(TabElement* tabElement);
    void CreatePageGraphics(TabElement* tabElement);
    void CreatePageAudio(TabElement* tabElement);
    void CreatePageInput(TabElement* tabElement);
    void CreatePageInterface(TabElement* tabElement);
    void LoadWindow(Window* wnd, const String& fileName);
    void FillShortcutsList();
    void HandleShortcutItemSelected(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    OptionsWindow(Context* context);
    ~OptionsWindow();
};

