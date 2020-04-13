/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "FwClient.h"
#include "AB/Entities/ConcreteItem.h"

class Item;

static const unsigned INVENTORY_COLS_PER_ROW = 5;
static const int INVENTORY_ITEM_SIZE_X = 48;
static const int INVENTORY_ITEM_SIZE_Y = 48;

class InventoryWindow : public Window
{
    URHO3D_OBJECT(InventoryWindow, Window)
private:
    bool initializted_;
    SharedPtr<Menu> itemPopup_;
    SharedPtr<Window> dragItem_;
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleInventory(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemUpdate(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemRemove(StringHash eventType, VariantMap& eventData);
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);
    void HandleItemDestroySelected(StringHash eventType, VariantMap& eventData);
    void HandleItemDropSelected(StringHash eventType, VariantMap& eventData);
    void HandleItemDragMove(StringHash eventType, VariantMap& eventData);
    void HandleItemDragBegin(StringHash eventType, VariantMap& eventData);
    void HandleItemDragCancel(StringHash eventType, VariantMap& eventData);
    void HandleItemDragEnd(StringHash eventType, VariantMap& eventData);
    BorderImage* GetItemContainer(uint16_t pos);
    void SetItem(Item* item, const InventoryItem& iItem);
    uint16_t GetItemPosFromClientPos(const IntVector2& clientPos);
public:
    static void RegisterObject(Context* context);

    InventoryWindow(Context* context);
    ~InventoryWindow() override;

    void GetInventory();
    void Clear();
    bool DropItem(const IntVector2& screenPos, AB::Entities::StoragePlace currentPlace, uint16_t currItemPos);
};

