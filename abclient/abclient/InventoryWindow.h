#pragma once

#include "Receiver.h"

class Item;

class InventoryWindow : public Window
{
    URHO3D_OBJECT(InventoryWindow, Window);
private:
    bool initializted_;
    SharedPtr<Menu> itemPopup_;
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleInventory(StringHash eventType, VariantMap& eventData);
    void HandleInventoryAdded(StringHash eventType, VariantMap& eventData);
    void HandleInventoryRemoved(StringHash eventType, VariantMap& eventData);
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);
    void HandleItemDestroySelected(StringHash eventType, VariantMap& eventData);
    BorderImage* GetItemContainer(uint16_t pos);
    void SetItem(Item* item, const Client::InventoryItem& iItem);
public:
    static void RegisterObject(Context* context);

    InventoryWindow(Context* context);
    ~InventoryWindow();

    void GetInventory();
};

