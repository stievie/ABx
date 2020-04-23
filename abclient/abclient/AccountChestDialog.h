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

#include "DialogWindow.h"
#include "AB/Entities/ConcreteItem.h"
#include "FwClient.h"

static const unsigned CHEST_COLS_PER_ROW = 10;
static const int CHEST_ITEM_SIZE_X = 48;
static const int CHEST_ITEM_SIZE_Y = 48;

class Item;
class NumberInputBox;

class AccountChestDialog : public DialogWindow
{
    URHO3D_OBJECT(AccountChestDialog, DialogWindow)
private:
    bool initializted_;
    SharedPtr<Window> dragItem_;
    SharedPtr<NumberInputBox> inputBox_;
    void HandleChest(StringHash eventType, VariantMap& eventData);
    void HandleChestItemUpdate(StringHash eventType, VariantMap& eventData);
    void HandleChestItemRemove(StringHash eventType, VariantMap& eventData);
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);
    void HandleItemDragMove(StringHash eventType, VariantMap& eventData);
    void HandleItemDragBegin(StringHash eventType, VariantMap& eventData);
    void HandleItemDragCancel(StringHash eventType, VariantMap& eventData);
    void HandleItemDragEnd(StringHash eventType, VariantMap& eventData);
    void HandleDepositClicked(StringHash eventType, VariantMap& eventData);
    void HandleWithdrawClicked(StringHash eventType, VariantMap& eventData);
    void HandleWithdrawDone(StringHash eventType, VariantMap& eventData);
    void HandleDepositDone(StringHash eventType, VariantMap& eventData);
    void HandleDialogClosed(StringHash eventType, VariantMap& eventData);
    uint16_t GetItemPosFromClientPos(const IntVector2& clientPos);
    BorderImage* GetItemContainer(uint16_t pos);
    void SetItem(Item* item, const ConcreteItem& iItem);
protected:
    void SubscribeEvents() override;
public:
    AccountChestDialog(Context* context);
    ~AccountChestDialog() override;
    void Initialize() override;
    bool DropItem(const IntVector2& screenPos, AB::Entities::StoragePlace currentPlace, uint16_t currItemPos);
    void Clear();

    uint32_t money_{ 0 };
};

