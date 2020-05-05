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

#include "stdafx.h"
#include "TradeDialog.h"
#include "Player.h"
#include "ItemsCache.h"
#include "NumberInputBox.h"
#include "ItemUIElement.h"

TradeDialog::TradeDialog(Context* context, SharedPtr<Player> player, SharedPtr<Actor> partner) :
    DialogWindow(context),
    player_(player),
    partner_(partner)
{
    LoadLayout("UI/TradeWindow.xml");
    SetStyleAuto();
    SetName("TradeDialog");

    SetLayoutSpacing(10);
    SetLayoutBorder({ 10, 10, 10, 10 });
    SetPosition({ 10, 10 });
    SetMovable(true);
    SetResizable(false);

    auto* caption = GetChildStaticCast<Text>("Caption", true);
    String title = "Trade with ";
    title.AppendWithFormat("%s", partner->name_.CString());
    caption->SetText(title);

    auto* partnerText = GetChildStaticCast<Text>("PartnersOfferText", true);
    String partnersOfferText;
    partnersOfferText.AppendWithFormat("%s's Offer", partner->name_.CString());
    partnerText->SetText(partnersOfferText);

    auto* receiveContainer = GetChild("ReceiveContainer", true);
    receiveContainer->SetVisible(false);

    auto* offerButton = GetChildStaticCast<Button>("OfferButton", true);
    SubscribeToEvent(offerButton, E_RELEASED, URHO3D_HANDLER(TradeDialog, HandleOfferClicked));
    offerButton->SetEnabled(true);
    auto* acceptButton = GetChildStaticCast<Button>("AcceptButton", true);
    SubscribeToEvent(acceptButton, E_RELEASED, URHO3D_HANDLER(TradeDialog, HandleAcceptClicked));
    acceptButton->SetEnabled(false);
    auto* cancelButton = GetChildStaticCast<Button>("CancelButton", true);
    SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(TradeDialog, HandleCancelClicked));
    auto* moneyEdit = GetChildStaticCast<LineEdit>("OfferMoneyEdit", true);
    moneyEdit->SetText("0");
    SubscribeToEvent(moneyEdit, E_TEXTENTRY, URHO3D_HANDLER(TradeDialog, HandleMoneyEditTextEntry));

    SubscribeToEvent(Events::E_TRADEOFFER, URHO3D_HANDLER(TradeDialog, HandlePartnersOffer));
    DialogWindow::SubscribeEvents();

    UpdateLayout();
    BringToFront();
}

TradeDialog::~TradeDialog()
{
}

bool TradeDialog::DropItem(const IntVector2& screenPos, ConcreteItem&& ci)
{
    if (ci.place != AB::Entities::StoragePlace::Inventory)
        return false;
    auto* itemsCache = GetSubsystem<ItemsCache>();
    auto item = itemsCache->Get(ci.index);
    if (!item)
        return false;
    if (!item->tradeAble_)
        return false;
    if (offered_)
        return false;

    if (ourOffer_.size() >= 7)
        return false;
    auto pos = ci.pos;
    if (ourOffer_.find(pos) != ourOffer_.end())
        return false;

    auto* container = GetChild("OfferItems", true);
    if (!container->IsInside(screenPos, true))
        return false;

    int freeSlot = FindFreeSlot(container);
    if (freeSlot == -1)
        return false;
    ItemUIElement* icon = CreateItem(container, freeSlot, ci);
    if (icon == nullptr)
        return false;

    uint32_t itemCount = ci.count;

    ourOffer_.emplace(pos, std::move(ci));

    if (itemCount > 1)
    {
        icon->SetVisible(false);
        ShowCountDialog(pos, itemCount);
    }
    return true;
}

void TradeDialog::HandleOfferClicked(StringHash, VariantMap&)
{
    uint32_t money = GetOfferedMoney();
    std::vector<std::pair<uint16_t, uint32_t>> ourItems;
    for (const auto& i : ourOffer_)
        ourItems.push_back({ i.first, i.second.count });

    auto* client = GetSubsystem<FwClient>();
    client->TradeOffer(money, std::move(ourItems));
    EnableOfferButton(false);
    auto* moneyEdit = GetChildStaticCast<LineEdit>("OfferMoneyEdit", true);
    moneyEdit->SetEnabled(false);
    offered_ = true;
    EnableAcceptButton();
}

void TradeDialog::HandleAcceptClicked(StringHash, VariantMap&)
{
    auto* acceptButton = GetChildStaticCast<Button>("AcceptButton", true);
    acceptButton->SetEnabled(false);
    auto* client = GetSubsystem<FwClient>();
    client->TradeAccept();
}

void TradeDialog::HandleCancelClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    client->TradeCancel();
    Close();
}

void TradeDialog::HandleMoneyEditTextEntry(StringHash, VariantMap& eventData)
{
    using namespace TextEntry;
    String text = eventData[P_TEXT].GetString();
    String newText;
    for (auto it = text.Begin(); it != text.End(); it++)
    {
        if (isdigit(*it))
            newText += (*it);
    }
    eventData[P_TEXT] = newText;
}

void TradeDialog::HandlePartnersOffer(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    const auto& offer = client->GetCurrentPartnersOffer();

    auto* moneyText = GetChildStaticCast<Text>("ReceiveMoneyText", true);
    moneyText->SetText(FormatMoney(offer.money) + " Drachma");

    auto* container = GetChild("ReceiveItems", true);
    int index = 0;
    for (const auto& item : offer.items)
    {
        ConcreteItem ci;
        ci.index = item.index;
        ci.count = item.count;
        ci.value = item.value;
        LoadStatsFromString(ci.stats, item.stats);
        CreateItem(container, index, ci);
        ++index;
    }
    gotOffer_ = true;
    EnableAcceptButton();
    auto* receiveContainer = GetChild("ReceiveContainer", true);
    receiveContainer->SetVisible(true);
}

void TradeDialog::HandleItemDragBegin(StringHash, VariantMap& eventData)
{
    if (offered_)
        // Once we have submitted our offer it's not possible to change it
        return;
    using namespace DragBegin;
    auto* item = reinterpret_cast<ItemUIElement*>(eventData[P_ELEMENT].GetVoidPtr());
    int buttons = eventData[P_BUTTONS].GetInt();
    int lx = eventData[P_X].GetInt();
    int ly = eventData[P_Y].GetInt();
    dragItem_ = item->GetDragItem(buttons, { lx, ly });
    dragItem_->BringToFront();
}

void TradeDialog::HandleItemDragMove(StringHash, VariantMap& eventData)
{
    if (!dragItem_)
        return;
    using namespace DragMove;
    dragItem_->BringToFront();

    int buttons = eventData[P_BUTTONS].GetInt();
    auto* element = reinterpret_cast<ItemUIElement*>(eventData[P_ELEMENT].GetVoidPtr());
    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();
    int BUTTONS = element->GetVar("BUTTONS").GetInt();

    if (buttons == BUTTONS)
        dragItem_->SetPosition(IntVector2(X, Y) - dragItem_->GetSize() / 2);
}

void TradeDialog::HandleItemDragCancel(StringHash, VariantMap&)
{
    using namespace DragCancel;
    if (!dragItem_)
        return;
    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragItem_.Get());
    dragItem_.Reset();
}

void TradeDialog::HandleItemDragEnd(StringHash, VariantMap& eventData)
{
    using namespace DragEnd;
    if (!dragItem_)
        return;
    uint16_t pos = static_cast<uint16_t>(dragItem_->GetVar("Pos").GetUInt());

    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();
    if (!IsInside({ X, Y }, true))
    {
        // If dropped outside of the trading dialog remove it
        auto* container = GetChild("OfferItems", true);
        if (RemoveItem(container, static_cast<unsigned>(pos)))
            ourOffer_.erase(pos);
    }

    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragItem_.Get());
    dragItem_.Reset();
}

void TradeDialog::HandleInputDialogClosed(StringHash, VariantMap&)
{
    if (inputBox_)
        inputBox_.Reset();
}

void TradeDialog::HandleInputDialogDone(StringHash, VariantMap& eventData)
{
    using namespace NumberInputBoxDone;
    NumberInputBox* dialog = static_cast<NumberInputBox*>(eventData[P_ELEMENT].GetVoidPtr());
    unsigned pos = dialog->GetVar("Pos").GetUInt();
    auto* container = GetChild("OfferItems", true);
    if (!eventData[P_OK].GetBool())
    {
        if (RemoveItem(container, pos))
            ourOffer_.erase(static_cast<uint16_t>(pos));
        return;
    }

    uint32_t count = eventData[P_VALUE].GetUInt();
    auto it = ourOffer_.find(static_cast<uint16_t>(pos));
    if (it == ourOffer_.end())
        return;

    (*it).second.count = count;
    auto* item = GetItemFromPos(container, pos);
    auto* itemElem = item->GetChildStaticCast<ItemUIElement>("ItemElememt", true);
    itemElem->SetCount(count);
    itemElem->SetVisible(true);
}

uint32_t TradeDialog::GetOfferedMoney() const
{
    auto* moneyEdit = GetChildStaticCast<LineEdit>("OfferMoneyEdit", true);
    const String& text = moneyEdit->GetText();
    const char* pVal = text.CString();
    char* pEnd;
    int iValue = strtol(pVal, &pEnd, 10);
    return static_cast<uint32_t>(iValue);
}

ItemUIElement* TradeDialog::CreateItem(UIElement* container, int index, const ConcreteItem& iItem)
{
    auto* itemsCache = GetSubsystem<ItemsCache>();
    auto item = itemsCache->Get(iItem.index);
    if (!item)
        return nullptr;

    auto* itemContainer = container->GetChild("Item" + String(index + 1), true);
    if (!itemContainer)
        return nullptr;

    ItemUIElement* itemElem = itemContainer->CreateChild<ItemUIElement>("ItemElememt");
    itemElem->SetPosition(4, 4);
    itemElem->SetSize(itemContainer->GetSize() - IntVector2(8, 8));
    itemElem->SetMinSize(itemElem->GetSize());
    itemElem->SetName(item->name_);
    itemElem->SetIcon(item->iconFile_);
    itemElem->SetPos(iItem.pos);
    itemElem->SetIndex(iItem.index);
    itemElem->SetCount(iItem.count);
    itemElem->SetValue(iItem.value);
    itemElem->SetStats(SaveStatsToString(iItem.stats));

    SubscribeToEvent(itemElem, E_DRAGMOVE, URHO3D_HANDLER(TradeDialog, HandleItemDragMove));
    SubscribeToEvent(itemElem, E_DRAGBEGIN, URHO3D_HANDLER(TradeDialog, HandleItemDragBegin));
    SubscribeToEvent(itemElem, E_DRAGCANCEL, URHO3D_HANDLER(TradeDialog, HandleItemDragCancel));
    SubscribeToEvent(itemElem, E_DRAGEND, URHO3D_HANDLER(TradeDialog, HandleItemDragEnd));

    return itemElem;
}

int TradeDialog::FindFreeSlot(UIElement* container)
{
    for (int i = 0; i < 7; ++i)
    {
        auto* itemContainer = container->GetChild("Item" + String(i + 1), true);
        if (!itemContainer)
            return -1;
        if (itemContainer->GetChildren().Size() == 0)
            return i;
    }
    return -1;
}

bool TradeDialog::RemoveItem(UIElement* container, unsigned pos)
{
    auto* item = GetItemFromPos(container, pos);
    if (item == nullptr)
        return false;

    item->RemoveAllChildren();
    return true;
}

UIElement* TradeDialog::GetItemFromPos(UIElement* container, unsigned pos)
{
    for (int i = 0; i < 7; ++i)
    {
        auto* itemContainer = container->GetChild("Item" + String(i + 1), true);
        if (!itemContainer)
            continue;
        ItemUIElement* icon = itemContainer->GetChildDynamicCast<ItemUIElement>("ItemElememt", true);
        if (!icon)
            continue;
        if (icon->pos_ == static_cast<unsigned>(pos))
            return itemContainer;
    }
    return nullptr;
}

void TradeDialog::EnableOfferButton(bool value)
{
    auto* offerButton = GetChildStaticCast<Button>("OfferButton", true);
    offerButton->SetEnabled(value);
}

void TradeDialog::EnableAcceptButton()
{
    if (offered_ && gotOffer_)
    {
        auto* acceptButton = GetChildStaticCast<Button>("AcceptButton", true);
        acceptButton->SetEnabled(true);
    }
}

void TradeDialog::ShowCountDialog(uint16_t pos, int max)
{
    if (inputBox_)
        inputBox_->Close();

    inputBox_ = MakeShared<NumberInputBox>(context_, "Number of Items");
    inputBox_->SetMax(max);
    inputBox_->SetVar("Pos", pos);
    inputBox_->SetShowMaxButton(true);
    inputBox_->SetMin(1);
    inputBox_->SetValue(1);
    inputBox_->SelectAll();
    SubscribeToEvent(inputBox_, E_NUMBERINPUTBOXDONE, URHO3D_HANDLER(TradeDialog, HandleInputDialogDone));
    SubscribeToEvent(inputBox_, E_DIALOGCLOSE, URHO3D_HANDLER(TradeDialog, HandleInputDialogClosed));
}
