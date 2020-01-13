#pragma once

#include <Urho3DAll.h>

class EquipmentWindow : public Window
{
    URHO3D_OBJECT(EquipmentWindow, Window)
private:
    void SubscribeEvents();
public:
    EquipmentWindow(Context* context);
    ~EquipmentWindow() override;

    void UpdateEquipment();
};

