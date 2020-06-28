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

#include "MerchantWindow.h"
#include "FwClient.h"
#include "ItemsCache.h"

MerchantWindow::MerchantWindow(Context* context) :
    DialogWindow(context)
{
    SetName(MerchantWindow::GetTypeNameStatic());

    LoadLayout("UI/MerchantWindow.xml");
    CreateUI();
    Center();

    SetStyleAuto();

    SubscribeEvents();
    Clear();
}

MerchantWindow::~MerchantWindow()
{
}

void MerchantWindow::CreateUI()
{
    doItButton_ = GetChildStaticCast<Button>("DoItButton", true);
    UIElement* countContainer = GetChild("CountContainer", true);
    countText_ = countContainer->GetChildStaticCast<LineEdit>("CountEdit", true);
    countSpinner_ = countContainer->CreateChild<Spinner>();
    countSpinner_->SetStyleAuto();
    countSpinner_->SetEdit(countText_);
    countSpinner_->SetFixedSize({ 22, 22 });
    countText_->SetVisible(false);
    countSpinner_->SetVisible(false);

    UIElement* container = GetChild("Container", true);
    tabgroup_ = container->CreateChild<TabGroup>();
    tabgroup_->SetSize(container->GetSize());
    tabgroup_->SetPosition(0, 0);

    tabgroup_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    tabgroup_->SetAlignment(HA_CENTER, VA_TOP);
    tabgroup_->SetColor(Color(0, 0, 0, 0));
    tabgroup_->SetStyleAuto();

    {
        TabElement* elem = CreateTab(tabgroup_, "Sell");
        CreatePageSell(elem);
    }
    {
        TabElement* elem = CreateTab(tabgroup_, "Buy");
        CreatePageBuy(elem);
    }

    UpdateSellList();

    SubscribeToEvent(sellItems_, E_ITEMSELECTED, URHO3D_HANDLER(MerchantWindow, HandleSellItemSelected));
    SubscribeToEvent(buyItems_, E_ITEMSELECTED, URHO3D_HANDLER(MerchantWindow, HandleBuyItemSelected));
    SubscribeToEvent(tabgroup_, E_TABSELECTED, URHO3D_HANDLER(MerchantWindow, HandleTabSelected));
    tabgroup_->SetEnabled(true);
    tabgroup_->SetSelectedIndex(0);
}

void MerchantWindow::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    Button* doitButton = GetChildStaticCast<Button>("DoItButton", true);
    SubscribeToEvent(doitButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleDoItClicked));
    Button* closeButton = GetChildStaticCast<Button>("GoodByeButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleGoodByeClicked));
    SubscribeToEvent(Events::E_MERCHANT_ITEMS, URHO3D_HANDLER(MerchantWindow, HandleMerchantItems));
    SubscribeToEvent(Events::E_INVENTORY, URHO3D_HANDLER(MerchantWindow, HandleInventory));
    SubscribeToEvent(Events::E_INVENTORYITEMUPDATE, URHO3D_HANDLER(MerchantWindow, HandleInventoryItemUpdate));
    SubscribeToEvent(Events::E_INVENTORYITEMDELETE, URHO3D_HANDLER(MerchantWindow, HandleInventoryItemRemove));
    SubscribeToEvent(Events::E_ITEM_PRICE, URHO3D_HANDLER(MerchantWindow, HandleItemPrice));
}

TabElement* MerchantWindow::CreateTab(TabGroup* tabs, const String& page)
{
    static const IntVector2 tabSize(65, 20);
    static const IntVector2 tabBodySize(500, 380);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    TabElement* tabElement = tabs->CreateTab(tabSize, tabBodySize);
    tabElement->tabText_->SetFont(cache->GetResource<Font>("Fonts/ClearSans-Regular.ttf"), 10);
    tabElement->tabText_->SetText(page);
    tabElement->tabBody_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());

    tabElement->tabBody_->SetImageRect(IntRect(48, 0, 64, 16));
    tabElement->tabBody_->SetLayoutMode(LM_VERTICAL);
    tabElement->tabBody_->SetPivot(0, 0);
    tabElement->tabBody_->SetPosition(0, 0);
    tabElement->tabBody_->SetStyleAuto();
    return tabElement;
}

void MerchantWindow::CreatePageSell(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    sellItems_ = page->CreateChild<ListView>("SellItemsListView");
    sellItems_->SetStyleAuto();
    sellItems_->SetHighlightMode(HM_ALWAYS);
    page->UpdateLayout();
}

void MerchantWindow::CreatePageBuy(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    buyItems_ = page->CreateChild<ListView>("BuyItemsListView");
    buyItems_->SetStyleAuto();
    buyItems_->SetHighlightMode(HM_ALWAYS);
    page->UpdateLayout();
}

void MerchantWindow::UpdateSellList()
{
    uint16_t oldSel = 0;
    auto sel = sellItems_->GetSelectedItem();
    if (sel)
        oldSel = static_cast<uint16_t>(sel->GetVar("Pos").GetUInt());

    sellItems_->RemoveAllItems();
    auto* client = GetSubsystem<FwClient>();
    auto* itemsCache = GetSubsystem<ItemsCache>();
    const auto& items = client->GetInventoryItems();
    unsigned index = 0;
    unsigned selIndex = M_MAX_UNSIGNED;

    std::vector<uint16_t> priceItems;
    for (const auto& ci : items)
    {
        auto item = itemsCache->Get(ci.index);
        if (item->type_ == AB::Entities::ItemType::Money)
            continue;
        if (!item->tradeAble_)
            continue;
        if (ci.value == 0 || ci.count == 0)
            continue;

        priceItems.push_back(ci.pos);
        CreateItem(*sellItems_, ci);
        if (oldSel != 0 && ci.pos == oldSel)
            selIndex = index;
        ++index;
    }
    if (selIndex != M_MAX_UNSIGNED)
        sellItems_->SetSelection(selIndex);

    client->GetItemPrice(priceItems);
}

void MerchantWindow::UpdateBuyList()
{
    uint16_t oldSel = 0;
    auto sel = buyItems_->GetSelectedItem();
    if (sel)
        oldSel = static_cast<uint16_t>(sel->GetVar("ID").GetUInt());

    buyItems_->RemoveAllItems();

    unsigned index = 0;
    unsigned selIndex = M_MAX_UNSIGNED;

    auto* client = GetSubsystem<FwClient>();
    const auto& items = client->GetMerchantItems();
    for (const auto& item : items)
    {
        CreateItem(*buyItems_, item);
        if (oldSel != 0 && item.id == oldSel)
            selIndex = index;
        ++index;
    }
    if (selIndex != M_MAX_UNSIGNED)
        buyItems_->SetSelection(selIndex);
}

UISelectable* MerchantWindow::GetSellItem(uint16_t pos)
{
    for (unsigned i = 0; i < sellItems_->GetNumItems(); ++i)
    {
        auto* item = sellItems_->GetItem(i);
        if (!item)
            continue;
        if (item->GetVar("Pos").GetUInt() == pos)
            return static_cast<UISelectable*>(item);
    }
    return nullptr;
}

unsigned MerchantWindow::GetSellItemIndex(uint16_t pos)
{
    for (unsigned i = 0; i < sellItems_->GetNumItems(); ++i)
    {
        auto* item = sellItems_->GetItem(i);
        if (!item)
            continue;
        if (item->GetVar("Pos").GetUInt() == pos)
            return i;
    }
    return M_MAX_UNSIGNED;
}

UISelectable* MerchantWindow::CreateItem(ListView& container, const ConcreteItem& iItem)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* itemsCache = GetSubsystem<ItemsCache>();
    auto item = itemsCache->Get(iItem.index);
    if (!item)
        return nullptr;

    UISelectable* uiS = container.CreateChild<UISelectable>("Item" + String(iItem.pos));
    uiS->SetLayoutSpacing(4);
    uiS->SetVar("ID", iItem.id);
    uiS->SetVar("Pos", iItem.pos);
    uiS->SetVar("Count", iItem.count);
    uiS->SetVar("Flags", iItem.flags);

    BorderImage* icon = uiS->CreateChild<BorderImage>("Icon");
    icon->SetInternal(true);

    UIElement* textContainer = uiS->CreateChild<UIElement>("TextContainer");
    textContainer->SetInternal(true);

    Text* nameElem = textContainer->CreateChild<Text>("Name");
    nameElem->SetText(item->name_);
    nameElem->SetInternal(true);
    Text* countElem = textContainer->CreateChild<Text>("Count");
    if (iItem.count > 1)
        countElem->SetText(String(iItem.count) + " x");
    countElem->SetInternal(true);

    Text* priceElem = textContainer->CreateChild<Text>("Price");
    priceElem->SetInternal(true);
    if (&container == sellItems_.Get())
    {
        priceElem->SetText(FormatMoney(iItem.value * iItem.count) + " D");
    }
    else
    {
        if (AB::Entities::IsItemStackable(iItem.flags))
            priceElem->SetText(FormatMoney(iItem.value) + " D each");
        else
            priceElem->SetText(FormatMoney(iItem.value) + " D");
    }

    uiS->SetStyle("MerchantListItem");
    Texture2D* texture = cache->GetResource<Texture2D>(item->iconFile_);
    icon->SetTexture(texture);
    icon->SetFullImageRect();
    container.AddItem(uiS);

    return uiS;
}

void MerchantWindow::ShowCountSpinner(bool b, uint32_t min, uint32_t max, uint32_t value)
{
    countText_->SetVisible(b);
    countSpinner_->SetVisible(b);
    if (min != 0)
        countSpinner_->SetMin(min);
    if (max != 0)
        countSpinner_->SetMax(max);
    if (value != 0)
        countSpinner_->SetValue(value);
}

void MerchantWindow::HandleMerchantItems(StringHash, VariantMap&)
{
    UpdateBuyList();
}

void MerchantWindow::HandleDoItClicked(StringHash, VariantMap&)
{
    if (tabgroup_->GetSelectedIndex() == 0)
    {
        // Sell
        auto* client = GetSubsystem<FwClient>();
        auto* item = sellItems_->GetSelectedItem();
        if (!item)
            return;
        uint16_t pos = static_cast<uint16_t>(item->GetVar("Pos").GetUInt());
        uint32_t count = 1;
        if (item->GetVar("Count").GetUInt() > 1)
            count = countSpinner_->GetValue();
        client->SellItem(npcId_, pos, count);
    }
    else
    {
        // Buy
        auto* client = GetSubsystem<FwClient>();
        auto* item = buyItems_->GetSelectedItem();
        if (!item)
            return;
        uint32_t id = item->GetVar("ID").GetUInt();
        uint32_t count = 1;
        if (item->GetVar("Count").GetUInt() > 1)
            count = countSpinner_->GetValue();
        client->BuyItem(npcId_, id, count);
        // TODO: Make this more selective not requesting the whole list again.
        client->RequestMerchantItems(npcId_);
    }
}

void MerchantWindow::HandleGoodByeClicked(StringHash, VariantMap&)
{
    npcId_ = 0;
    Close();
}

void MerchantWindow::HandleInventory(StringHash, VariantMap&)
{
    UpdateSellList();
}

void MerchantWindow::HandleInventoryItemUpdate(StringHash, VariantMap&)
{
    UpdateSellList();
}

void MerchantWindow::HandleInventoryItemRemove(StringHash, VariantMap& eventData)
{
    using namespace Events::InventoryItemDelete;
    uint16_t pos = static_cast<uint16_t>(eventData[P_ITEMPOS].GetUInt());
    unsigned itemIndex = GetSellItemIndex(pos);
    if (itemIndex != M_MAX_UNSIGNED)
        sellItems_->RemoveItem(itemIndex);
}

void MerchantWindow::HandleTabSelected(StringHash, VariantMap& eventData)
{
    ShowCountSpinner(false);

    using namespace TabSelected;
    int index = eventData[P_INDEX].GetInt();
    if (index == 0)
    {
        Text* txt = doItButton_->GetChildStaticCast<Text>("DoItButtonText", true);
        txt->SetText("Sell");
        UpdateSellList();
    }
    else
    {
        Text* txt = doItButton_->GetChildStaticCast<Text>("DoItButtonText", true);
        txt->SetText("Buy");
        auto* client = GetSubsystem<FwClient>();
        client->RequestMerchantItems(npcId_);
    }
    UpdateLayout();
}

void MerchantWindow::HandleSellItemSelected(StringHash, VariantMap& eventData)
{
    using namespace ItemSelected;
    unsigned index = eventData[P_SELECTION].GetUInt();
    auto* item = sellItems_->GetItem(index);
    if (index == M_MAX_UNSIGNED || !item)
    {
        ShowCountSpinner(false);
        return;
    }

    unsigned count = item->GetVar("Count").GetUInt();
    if (count > 1)
        ShowCountSpinner(true, 1, count, count);
    else
        ShowCountSpinner(false);
}

void MerchantWindow::HandleBuyItemSelected(StringHash, VariantMap& eventData)
{
    using namespace ItemSelected;
    unsigned index = eventData[P_SELECTION].GetUInt();
    auto* item = buyItems_->GetItem(index);
    if (index == M_MAX_UNSIGNED || !item)
    {
        ShowCountSpinner(false);
        return;
    }

    unsigned flags = item->GetVar("Flags").GetUInt();
    if (AB::Entities::IsItemStackable(flags))
        ShowCountSpinner(true, 1, 10, 10);
    else
        ShowCountSpinner(false);
}

void MerchantWindow::HandleItemPrice(StringHash, VariantMap& eventData)
{
    using namespace Events::ItemPrice;
    auto* item = GetSellItem(static_cast<uint16_t>(eventData[P_ITEMPOS].GetUInt()));
    if (!item)
        return;

    unsigned price = eventData[P_PRICE].GetUInt();
    unsigned count = item->GetVar("Count").GetUInt();
    Text* priceText = item->GetChildStaticCast<Text>("Price", true);
    String txt;
    if (count > 1)
        txt.AppendWithFormat("%s D (%u D each)", FormatMoney(price * count).CString(), price);
    else
        txt.AppendWithFormat("%s D", FormatMoney(price).CString());
    priceText->SetText(txt);
}

void MerchantWindow::Initialize(uint32_t npcId)
{
    Clear();
    npcId_ = npcId;
    auto* client = GetSubsystem<FwClient>();
    client->UpdateInventory();
}

void MerchantWindow::Clear()
{
    npcId_ = 0;
    sellItems_->RemoveAllItems();
    buyItems_->RemoveAllItems();
}
