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

    auto* caption = GetChildDynamicCast<Text>("Caption", true);
    String title = "Trade with ";
    title.AppendWithFormat("%s", partner->name_.CString());
    caption->SetText(title);

    auto* partnerText = GetChildDynamicCast<Text>("PartnersOfferText", true);
    String partnersOfferText;
    partnersOfferText.AppendWithFormat("%s's Offer", partner->name_.CString());
    partnerText->SetText(partnersOfferText);

    auto* okButton = GetChildDynamicCast<Button>("OfferButton", true);
    SubscribeToEvent(okButton, E_RELEASED, URHO3D_HANDLER(TradeDialog, HandleOfferClicked));
    auto* acceptButton = GetChildDynamicCast<Button>("AcceptButton", true);
    SubscribeToEvent(acceptButton, E_RELEASED, URHO3D_HANDLER(TradeDialog, HandleAcceptClicked));
    auto* cancelButton = GetChildDynamicCast<Button>("CancelButton", true);
    SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(TradeDialog, HandleCancelClicked));
    auto* moneyEdit = GetChildDynamicCast<LineEdit>("OfferMoneyEdit", true);
    moneyEdit->SetText("0");
    SubscribeToEvent(moneyEdit, E_TEXTENTRY, URHO3D_HANDLER(TradeDialog, HandleMoneyEditTextEntry));

    SubscribeToEvent(Events::E_TRADEOFFER, URHO3D_HANDLER(TradeDialog, HandlePartnersOffer));

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
    if (!CreateItem(container, freeSlot, ci))
        return false;

    ourOffer_.emplace(pos, std::move(ci));
    return true;
}

void TradeDialog::HandleOfferClicked(StringHash, VariantMap&)
{
    uint32_t money = GetOfferedMoney();
    if (money == 0 && ourOffer_.size() == 0)
        return;

    std::vector<uint16_t> ourItems;
    for (auto i : ourOffer_)
        ourItems.push_back(i.first);

    auto* client = GetSubsystem<FwClient>();
    client->TradeOffer(money, std::move(ourItems));
}

void TradeDialog::HandleAcceptClicked(StringHash, VariantMap&)
{
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
}

void TradeDialog::HandleItemDragBegin(StringHash, VariantMap& eventData)
{
    using namespace DragBegin;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    auto* item = reinterpret_cast<Button*>(eventData[P_ELEMENT].GetVoidPtr());
    UIElement* root = GetSubsystem<UI>()->GetRoot();

    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    dragItem_ = root->CreateChild<Window>();
    dragItem_->SetLayout(LM_HORIZONTAL);
    dragItem_->SetLayoutBorder(IntRect(4, 4, 4, 4));
    dragItem_->SetTexture(tex);
    dragItem_->SetImageRect(IntRect(48, 0, 64, 16));
    dragItem_->SetBorder(IntRect(4, 4, 4, 4));
    dragItem_->SetMinSize(item->GetSize());
    dragItem_->SetMaxSize(item->GetSize());
    BorderImage* icon = dragItem_->CreateChild<BorderImage>();
    icon->SetTexture(item->GetTexture());
    dragItem_->SetPosition(item->GetPosition());
    dragItem_->SetVar("POS", item->GetVar("POS"));
    dragItem_->SetVar("Index", item->GetVar("Index"));
    dragItem_->SetVar("Count", item->GetVar("Count"));
    dragItem_->SetVar("Value", item->GetVar("Value"));
    dragItem_->SetVar("Stats", item->GetVar("Stats"));

    int lx = eventData[P_X].GetInt();
    int ly = eventData[P_Y].GetInt();
    dragItem_->SetPosition(IntVector2(lx, ly) - dragItem_->GetSize() / 2);

    int buttons = eventData[P_BUTTONS].GetInt();
    item->SetVar("BUTTONS", buttons);
    dragItem_->BringToFront();
}

void TradeDialog::HandleItemDragMove(StringHash, VariantMap& eventData)
{
    if (!dragItem_)
        return;
    using namespace DragMove;
    dragItem_->BringToFront();

    int buttons = eventData[P_BUTTONS].GetInt();
    auto* element = reinterpret_cast<Button*>(eventData[P_ELEMENT].GetVoidPtr());
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
    uint16_t pos = static_cast<uint16_t>(dragItem_->GetVar("POS").GetUInt());

    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();
    if (!IsInside({ X, Y }, true))
    {
        // If dropped outside of the trading dialog remove it
        auto* container = GetChild("OfferItems", true);
        if (RemoveItem(container, static_cast<int>(pos)))
        {
            ourOffer_.erase(pos);
        }
    }

    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragItem_.Get());
    dragItem_.Reset();
}

uint32_t TradeDialog::GetOfferedMoney() const
{
    auto* moneyEdit = GetChildDynamicCast<LineEdit>("OfferMoneyEdit", true);
    const String& text = moneyEdit->GetText();
    const char* pVal = text.CString();
    char* pEnd;
    int iValue = strtol(pVal, &pEnd, 10);
    return static_cast<uint32_t>(iValue);
}

bool TradeDialog::CreateItem(UIElement* container, int index, const ConcreteItem& iItem)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    auto* itemsCache = GetSubsystem<ItemsCache>();
    auto item = itemsCache->Get(iItem.index);
    if (!item)
        return false;

    auto* itemContainer = container->GetChild("Item" + String(index + 1), true);
    if (!itemContainer)
        return false;

    // For ToolTips we need a button
    Button* icon = itemContainer->CreateChild<Button>("Icon");
    icon->SetPosition(4, 4);
    icon->SetSize(itemContainer->GetSize() - IntVector2(8, 8));
    icon->SetMinSize(icon->GetSize());
    Texture2D* texture = cache->GetResource<Texture2D>(item->iconFile_);
    icon->SetTexture(texture);
    icon->SetFullImageRect();
    icon->SetLayoutMode(LM_FREE);
    icon->SetVar("POS", iItem.pos);
    icon->SetVar("Index", iItem.index);
    icon->SetVar("Count", iItem.count);
    icon->SetVar("Value", iItem.value);
    icon->SetVar("Stats", SaveStatsToString(iItem.stats));

    SubscribeToEvent(icon, E_DRAGMOVE, URHO3D_HANDLER(TradeDialog, HandleItemDragMove));
    SubscribeToEvent(icon, E_DRAGBEGIN, URHO3D_HANDLER(TradeDialog, HandleItemDragBegin));
    SubscribeToEvent(icon, E_DRAGCANCEL, URHO3D_HANDLER(TradeDialog, HandleItemDragCancel));
    SubscribeToEvent(icon, E_DRAGEND, URHO3D_HANDLER(TradeDialog, HandleItemDragEnd));

    if (iItem.count > 1)
    {
        Text* count = icon->CreateChild<Text>("Count");
        count->SetAlignment(HA_LEFT, VA_BOTTOM);
        count->SetPosition(0, 0);
        count->SetSize(10, icon->GetWidth());
        count->SetMinSize(10, icon->GetWidth());
        count->SetText(String(iItem.count));
        count->SetStyleAuto();                  // !!!
        count->SetFontSize(9);
    }

    {
        // Tooltip
        ToolTip* tt = icon->CreateChild<ToolTip>();
        tt->SetLayoutMode(LM_HORIZONTAL);
        Window* ttWindow = tt->CreateChild<Window>();
        ttWindow->SetLayoutMode(LM_VERTICAL);
        ttWindow->SetLayoutBorder(IntRect(4, 4, 4, 4));
        ttWindow->SetStyleAuto();
        Text* ttText1 = ttWindow->CreateChild<Text>();
        String text = iItem.count > 1 ? String(iItem.count) + " " : "";
        text += item->name_;
        ttText1->SetText(text);
        ttText1->SetStyleAuto();

        String text2 = String(iItem.count * iItem.value) + " Drachma";
        Text* ttText2 = ttWindow->CreateChild<Text>();
        ttText2->SetText(text2);
        ttText2->SetStyleAuto();
        ttText2->SetFontSize(9);

        tt->SetPriority(2147483647);
        tt->SetOpacity(0.7f);
        tt->SetStyleAuto();
        tt->SetPosition(IntVector2(0, -(ttWindow->GetHeight() + 10)));
    }
    return true;
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

bool TradeDialog::RemoveItem(UIElement* container, int pos)
{
    for (int i = 0; i < 7; ++i)
    {
        auto* itemContainer = container->GetChild("Item" + String(i + 1), true);
        if (!itemContainer)
            continue;
        Button* icon = itemContainer->GetChildStaticCast<Button>("Icon", true);
        if (!icon)
            continue;
        if (icon->GetVar("POS").GetUInt() == static_cast<unsigned>(pos))
        {
            itemContainer->RemoveAllChildren();
            return true;
        }
    }
    return false;
}
