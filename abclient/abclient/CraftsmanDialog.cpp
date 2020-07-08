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

#include "CraftsmanDialog.h"

CraftsmanDialog::CraftsmanDialog(Context* context) :
    DialogWindow(context)
{
    SetName(CraftsmanDialog::GetTypeNameStatic());

    SetMinSize({ 438, 461 });
    LoadLayout("UI/CraftsmanWindow.xml");
    CreateUI();
    Center();

    SetStyleAuto();
    UpdateLayout();

    SubscribeEvents();
    Clear();
}

CraftsmanDialog::~CraftsmanDialog()
{ }

void CraftsmanDialog::CreateUI()
{
    itemTypesList_ = GetChildStaticCast<DropDownList>("ItemTypeList", true);
    itemTypesList_->SetResizePopup(true);
    SubscribeToEvent(itemTypesList_, E_ITEMSELECTED, URHO3D_HANDLER(CraftsmanDialog, HandleItemTypeSelected));
    searchNameEdit_ = GetChildStaticCast<LineEdit>("SearchTextBox", true);
    SubscribeToEvent(searchNameEdit_, E_TEXTFINISHED, URHO3D_HANDLER(CraftsmanDialog, HandleSearchItemEditTextFinished));
    auto* searchButton = GetChildStaticCast<Button>("SearchButton", true);
    SubscribeToEvent(searchButton, E_RELEASED, URHO3D_HANDLER(CraftsmanDialog, HandleSearchButtonClicked));

    currentPage_ = GetChildStaticCast<Text>("CurrentPageText", true);
    auto* firstButton = GetChildStaticCast<Button>("FirstPageButton", true);
    SubscribeToEvent(firstButton, E_RELEASED, URHO3D_HANDLER(CraftsmanDialog, HandleFirstPageButtonClicked));
    auto* prevButton = GetChildStaticCast<Button>("PrevPageButton", true);
    SubscribeToEvent(prevButton, E_RELEASED, URHO3D_HANDLER(CraftsmanDialog, HandlePrevPageButtonClicked));
    auto* nextButton = GetChildStaticCast<Button>("NextPageButton", true);
    SubscribeToEvent(nextButton, E_RELEASED, URHO3D_HANDLER(CraftsmanDialog, HandleNextPageButtonClicked));
    auto* lastButton = GetChildStaticCast<Button>("LastPageButton", true);
    SubscribeToEvent(lastButton, E_RELEASED, URHO3D_HANDLER(CraftsmanDialog, HandleLastPageButtonClicked));

    auto* listContainer = GetChild("ListContainer", true);
    listContainer->SetLayoutMode(LM_VERTICAL);
    items_ = listContainer->CreateChild<ListView>("ItemsListView");
    items_->SetStyleAuto();
    items_->SetHighlightMode(HM_ALWAYS);
}

void CraftsmanDialog::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    Button* doitButton = GetChildStaticCast<Button>("DoItButton", true);
    SubscribeToEvent(doitButton, E_RELEASED, URHO3D_HANDLER(CraftsmanDialog, HandleDoItClicked));
    Button* closeButton = GetChildStaticCast<Button>("GoodByeButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(CraftsmanDialog, HandleGoodByeClicked));
}

void CraftsmanDialog::HandleDoItClicked(StringHash, VariantMap&)
{
}

void CraftsmanDialog::HandleGoodByeClicked(StringHash, VariantMap&)
{
    npcId_ = 0;
    Close();
}

void CraftsmanDialog::HandlePrevPageButtonClicked(StringHash, VariantMap&)
{
}

void CraftsmanDialog::HandleNextPageButtonClicked(StringHash, VariantMap&)
{
}

void CraftsmanDialog::HandleFirstPageButtonClicked(StringHash, VariantMap&)
{
}

void CraftsmanDialog::HandleLastPageButtonClicked(StringHash, VariantMap&)
{
}

void CraftsmanDialog::HandleSearchButtonClicked(StringHash, VariantMap&)
{
}

void CraftsmanDialog::HandleItemTypeSelected(StringHash, VariantMap&)
{
}

void CraftsmanDialog::HandleSearchItemEditTextFinished(StringHash, VariantMap&)
{
}

void CraftsmanDialog::Initialize(uint32_t npcId)
{
    Clear();
    npcId_ = npcId;
}

void CraftsmanDialog::Clear()
{
}

