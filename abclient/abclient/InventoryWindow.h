#pragma once

class InventoryWindow : public Window
{
    URHO3D_OBJECT(InventoryWindow, Window);
private:
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleInventory(StringHash eventType, VariantMap& eventData);
    BorderImage* GetItemContainer(uint16_t pos);
public:
    static void RegisterObject(Context* context);

    InventoryWindow(Context* context);
    ~InventoryWindow();

    void GetInventory();
};

