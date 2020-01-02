#pragma once

#include "FwClient.h"

class Item;

class InventoryWindow : public Window
{
    URHO3D_OBJECT(InventoryWindow, Window)
private:
    bool initializted_;
    SharedPtr<Menu> itemPopup_;
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleInventory(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemUpdate(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemRemove(StringHash eventType, VariantMap& eventData);
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);
    void HandleItemStoreSelected(StringHash eventType, VariantMap& eventData);
    void HandleItemDestroySelected(StringHash eventType, VariantMap& eventData);
    void HandleItemDropSelected(StringHash eventType, VariantMap& eventData);
    BorderImage* GetItemContainer(uint16_t pos);
    void SetItem(Item* item, const InventoryItem& iItem);
public:
    static void RegisterObject(Context* context);

    InventoryWindow(Context* context);
    ~InventoryWindow() override;

    void GetInventory();
    void Clear();
};

