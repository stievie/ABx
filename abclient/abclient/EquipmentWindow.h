#pragma once

#include <Urho3D/UI/Window.h>

class EquipmentWindow : public Window
{
    URHO3D_OBJECT(EquipmentWindow, Window)
private:
    void CreateUI();
    Text* CreateDropdownItem(const String& text, const String& value);
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleProfessionSelected(StringHash eventType, VariantMap& eventData);
public:
    EquipmentWindow(Context* context);
    ~EquipmentWindow() override;

    void UpdateEquipment();
};
