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

    auto* container = GetChild("OfferItems", true);
    if (!container->IsInside(screenPos, true))
        return false;

    auto pos = ci.pos;
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
    (void)offer;
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
