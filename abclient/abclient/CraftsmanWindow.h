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

class CraftsmanWindow final : public DialogWindow
{
    URHO3D_OBJECT(CraftsmanWindow, DialogWindow)
private:
    uint32_t npcId_{ 0 };
    SharedPtr<ListView> items_;
    SharedPtr<DropDownList> itemTypesList_;
    SharedPtr<LineEdit> searchNameEdit_;
    SharedPtr<Text> currentPage_;
    SharedPtr<DropDownList> attribDropdown_;
    SharedPtr<UIElement> pagingContainer_;
    void CreateUI();
    void SubscribeEvents();
    void UpdateList();
    void RequestList(uint32_t page = 1, bool keepScrollPos = false);
    AB::Entities::ItemType GetSelectedItemType() const;
    uint32_t GetSelectedItemIndex() const;
    UISelectable* CreateItem(const ConcreteItem& iItem);
    void PopulateItemTypes();
    void UpdateAttributeList();
    uint32_t GetSelectedAttributeIndex() const;
    void HandleCraftsmanItems(StringHash eventType, VariantMap& eventData);
    void HandleDoItClicked(StringHash eventType, VariantMap& eventData);
    void HandleGoodByeClicked(StringHash eventType, VariantMap& eventData);
    void HandleItemTypeSelected(StringHash eventType, VariantMap& eventData);
    void HandleItemSelected(StringHash eventType, VariantMap& eventData);
    void HandleSearchItemEditTextFinished(StringHash eventType, VariantMap& eventData);
    void HandlePrevPageButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleNextPageButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleFirstPageButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleLastPageButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleSearchButtonClicked(StringHash eventType, VariantMap& eventData);
public:
    CraftsmanWindow(Context* context);
    ~CraftsmanWindow() override;
    void Initialize(uint32_t npcId) override;
    void Clear();
};

