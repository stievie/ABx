#pragma once

#include <Urho3DAll.h>

class Player;

class EquipmentWindow : public Window
{
    URHO3D_OBJECT(EquipmentWindow, Window)
private:
    void CreateUI();
    void AddProfessions(const Player& player);
    void UpdateAttributes(const Player& player);
    void UpdateSkills(const Player& player);
    void UpdateEquipment(const Player& player);
    Text* CreateDropdownItem(const String& text, unsigned value);
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleProfessionSelected(StringHash eventType, VariantMap& eventData);
public:
    EquipmentWindow(Context* context);
    ~EquipmentWindow() override;

    void UpdateAll();
};
