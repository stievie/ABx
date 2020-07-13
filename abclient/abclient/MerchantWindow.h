/**
 * Copyright 2020 Stefan Ascher
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
#include "TabGroup.h"
#include "Spinner.h"

class MerchantWindow final : public DialogWindow
{
    URHO3D_OBJECT(MerchantWindow, DialogWindow)
private:
    SharedPtr<TabGroup> tabgroup_;
    SharedPtr<ListView> sellItems_;
    SharedPtr<ListView> buyItems_;
    SharedPtr<LineEdit> countText_;
    SharedPtr<Spinner> countSpinner_;
    SharedPtr<Button> doItButton_;
    SharedPtr<LineEdit> searchNameEdit_;
    SharedPtr<DropDownList> itemTypesList_;
    SharedPtr<Text> currentPage_;
    SharedPtr<UIElement> pagingContainer_;
    uint32_t npcId_{ 0 };
    void CreateUI();
    void SubscribeEvents();
    TabElement* CreateTab(TabGroup* tabs, const String& page);
    void CreatePageSell(TabElement* tabElement);
    void CreatePageBuy(TabElement* tabElement);
    void UpdateSellList();
    void UpdateBuyList();
    UISelectable* GetSellItem(uint16_t pos);
    unsigned GetSellItemIndex(uint16_t pos) const;
    UISelectable* CreateItem(ListView& container, const ConcreteItem& iItem);
    uint16_t GetSelectedItemType() const;
    void RequestBuyItems(uint32_t page = 1, bool keepScrollPos = false);
    void ShowCountSpinner(bool b, uint32_t min = 0, uint32_t max = 0, uint32_t value = 0);
    void PopulateBuyItemTypes();
    void HandleMerchantItems(StringHash eventType, VariantMap& eventData);
    void HandleDoItClicked(StringHash eventType, VariantMap& eventData);
    void HandleGoodByeClicked(StringHash eventType, VariantMap& eventData);
    void HandleInventory(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemUpdate(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemRemove(StringHash eventType, VariantMap& eventData);
    void HandleTabSelected(StringHash eventType, VariantMap& eventData);
    void HandleSellItemSelected(StringHash eventType, VariantMap& eventData);
    void HandleBuyItemSelected(StringHash eventType, VariantMap& eventData);
    void HandleItemPrice(StringHash eventType, VariantMap& eventData);
    void HandleSearchButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleItemTypeSelected(StringHash eventType, VariantMap& eventData);
    void HandleSearchItemEditTextFinished(StringHash eventType, VariantMap& eventData);
    void HandlePrevPageButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleNextPageButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleFirstPageButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleLastPageButtonClicked(StringHash eventType, VariantMap& eventData);
public:
    MerchantWindow(Context* context);
    ~MerchantWindow() override;
    void Initialize(uint32_t npcId) override;
    void Clear();
};

