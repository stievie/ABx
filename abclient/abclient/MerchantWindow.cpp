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
#include "ItemStatsUIElement.h"

MerchantWindow::MerchantWindow(Context* context) :
    DialogWindow(context)
{
    SetName(MerchantWindow::GetTypeNameStatic());

    DisableLayoutUpdate();
    SetMinSize({ 438, 461 });
    LoadLayout("UI/MerchantWindow.xml");
    CreateUI();
    Center();

    SetStyleAuto();
    SetResizable(true);
    EnableLayoutUpdate();
    UpdateLayout();
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
    countSpinner_->SetEdit(countText_);
    countSpinner_->SetFixedHeight(22);
    countText_->SetVisible(false);
    countSpinner_->SetValue(10);
    countSpinner_->SetStyleAuto();
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
}

void MerchantWindow::CreatePageBuy(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    page->SetLayoutMode(LM_VERTICAL);
    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/MerchantWindowBuyPage.xml");
    wnd->SetPosition(0, 0);

    itemTypesList_ = wnd->GetChildStaticCast<DropDownList>("ItemTypeList", true);
    itemTypesList_->SetResizePopup(true);
    SubscribeToEvent(itemTypesList_, E_ITEMSELECTED, URHO3D_HANDLER(MerchantWindow, HandleItemTypeSelected));

    searchNameEdit_ = wnd->GetChildStaticCast<LineEdit>("SearchTextBox", true);
    SubscribeToEvent(searchNameEdit_, E_TEXTFINISHED, URHO3D_HANDLER(MerchantWindow, HandleSearchItemEditTextFinished));

    auto* searchButton = wnd->GetChildStaticCast<Button>("SearchButton", true);
    SubscribeToEvent(searchButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleSearchButtonClicked));

    pagingContainer_ = wnd->GetChild("PageContainer", true);
    currentPage_ = pagingContainer_->GetChildStaticCast<Text>("CurrentPageText", true);
    auto* firstButton = pagingContainer_->GetChildStaticCast<Button>("FirstPageButton", true);
    SubscribeToEvent(firstButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleFirstPageButtonClicked));
    auto* prevButton = pagingContainer_->GetChildStaticCast<Button>("PrevPageButton", true);
    SubscribeToEvent(prevButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandlePrevPageButtonClicked));
    auto* nextButton = pagingContainer_->GetChildStaticCast<Button>("NextPageButton", true);
    SubscribeToEvent(nextButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleNextPageButtonClicked));
    auto* lastButton = pagingContainer_->GetChildStaticCast<Button>("LastPageButton", true);
    SubscribeToEvent(lastButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleLastPageButtonClicked));

    wnd->SetLayoutMode(LM_VERTICAL);

    auto* listContainer = wnd->GetChild("MerchantListContainer", true);
    listContainer->SetLayoutMode(LM_VERTICAL);
    buyItems_ = listContainer->CreateChild<ListView>("BuyItemsListView");
    buyItems_->SetStyleAuto();
    buyItems_->SetHighlightMode(HM_ALWAYS);
}

void MerchantWindow::UpdateSellList()
{
    uint16_t oldSel = 0;
    auto sel = sellItems_->GetSelectedItem();
    if (sel)
        oldSel = static_cast<uint16_t>(sel->GetVar("Pos").GetUInt());

    const auto scrollPos = sellItems_->GetViewPosition();
    sellItems_->DisableLayoutUpdate();
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
    sellItems_->EnableLayoutUpdate();
    sellItems_->SetViewPosition(scrollPos);
    client->GetItemPrice(priceItems);
}

void MerchantWindow::UpdateBuyList()
{
    PopulateBuyItemTypes();
    uint32_t oldSel = 0;
    auto sel = buyItems_->GetSelectedItem();
    if (sel)
        oldSel = sel->GetVar("ID").GetUInt();

    const auto scrollPos = buyItems_->GetViewPosition();
    buyItems_->DisableLayoutUpdate();
    buyItems_->RemoveAllItems();

    unsigned index = 0;
    unsigned selIndex = M_MAX_UNSIGNED;

    auto* client = GetSubsystem<FwClient>();
    String pagingText;
    pagingText.AppendWithFormat("%d/%d", client->GetMerchantItemsPage(), client->GetMerchantItemsPageCount());
    currentPage_->SetText(pagingText);
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
    buyItems_->EnableLayoutUpdate();
    buyItems_->SetViewPosition(scrollPos);
    pagingContainer_->SetVisible(client->GetMerchantItemsPageCount() > 1);
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

unsigned MerchantWindow::GetSellItemIndex(uint16_t pos) const
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

    UISelectable* uiS = container.CreateChild<UISelectable>("Item" + String(iItem.id));
    uiS->SetLayoutSpacing(4);
    uiS->SetVar("ID", iItem.id);
    uiS->SetVar("Pos", iItem.pos);
    uiS->SetVar("Count", iItem.count);
    uiS->SetVar("Flags", iItem.flags);
    uiS->SetVar("Index", iItem.index);

    BorderImage* icon = uiS->CreateChild<BorderImage>("Icon");
    icon->SetInternal(true);

    UIElement* textContainer = uiS->CreateChild<UIElement>("TextContainer");
    textContainer->SetInternal(true);
    Text* nameElem = textContainer->CreateChild<Text>("Name");
    nameElem->SetText(item->name_);
    nameElem->SetInternal(true);

    if (!iItem.stats.Empty())
    {
        auto* itemStats = textContainer->CreateChild<ItemStatsUIElement>();
        itemStats->SetInternal(true);
        itemStats->SetStats(iItem.stats);
    }

    Text* countElem = uiS->CreateChild<Text>("Count");
    if (iItem.count > 1)
        countElem->SetText(String(iItem.count) + " x");
    countElem->SetInternal(true);

    Text* priceElem = uiS->CreateChild<Text>("Price");
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

uint16_t MerchantWindow::GetSelectedItemType() const
{
    auto* item = itemTypesList_->GetSelectedItem();
    if (!item)
        return 0;

    unsigned type = item->GetVar("Type").GetUInt();
    return static_cast<uint16_t>(type);
}

void MerchantWindow::RequestBuyItems(uint32_t page, bool keepScrollPos)
{
    if (!keepScrollPos)
        buyItems_->SetViewPosition({ 0, 0 });
    auto* client = GetSubsystem<FwClient>();
    const String& search = searchNameEdit_->GetText();
    client->RequestMerchantItems(npcId_, GetSelectedItemType(), search, page);
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
    countText_->SetChildOffset({ 0, 0 });
    countText_->GetParent()->UpdateLayout();
}

void MerchantWindow::PopulateBuyItemTypes()
{
    auto createDropdownItem = [this](const String& text, AB::Entities::ItemType type)
    {
        Text* result = new Text(context_);
        result->SetText(text);
        result->SetStyle("DropDownItemEnumText");
        result->SetVar("Type", static_cast<unsigned>(type));
        return result;
    };

    uint16_t sel = GetSelectedItemType();
    itemTypesList_ = GetChildStaticCast<DropDownList>("ItemTypeList", true);
    itemTypesList_->RemoveAllItems();

    auto* client = GetSubsystem<FwClient>();
    itemTypesList_->AddItem(createDropdownItem("(Everything)", static_cast<AB::Entities::ItemType>(0)));
    const auto& types = client->GetMerchantItemTypes();
    if (types.size() > 1)
    {
        unsigned selected = 0;
        for (auto type : types)
        {
            String text = FwClient::GetItemTypeName(type);
            if (text.Empty())
                continue;
            if (type == static_cast<AB::Entities::ItemType>(sel))
                selected = itemTypesList_->GetNumItems();
            itemTypesList_->AddItem(createDropdownItem(text, type));
        }
        itemTypesList_->SetSelection(selected);
        itemTypesList_->SetVisible(true);
    }
    else
    {
        // If there is just one type we can hide it as well, because there isn't much to choose from.
        itemTypesList_->SetSelection(0);
        itemTypesList_->SetVisible(false);
    }
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
        if (countSpinner_->IsVisible())
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
        if (countSpinner_->IsVisible())
            count = countSpinner_->GetValue();
        client->BuyItem(npcId_, id, count);
        // TODO: Make this more selective not requesting the whole list again.
        RequestBuyItems(client->GetMerchantItemsPage(), true);
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
        countSpinner_->SetValue(10);
        RequestBuyItems();
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
        ShowCountSpinner(true, 1, 250, 0);
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

void MerchantWindow::HandleSearchButtonClicked(StringHash, VariantMap&)
{
    RequestBuyItems();
}

void MerchantWindow::HandleItemTypeSelected(StringHash, VariantMap&)
{
    RequestBuyItems();
}

void MerchantWindow::HandleSearchItemEditTextFinished(StringHash, VariantMap&)
{
    RequestBuyItems();
}

void MerchantWindow::HandlePrevPageButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    if (client->GetMerchantItemsPage() < 2)
        return;
    RequestBuyItems(client->GetMerchantItemsPage() - 1);
}

void MerchantWindow::HandleNextPageButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    uint32_t page = client->GetMerchantItemsPage() + 1;
    if (page <= client->GetMerchantItemsPageCount())
        RequestBuyItems(page);
}

void MerchantWindow::HandleFirstPageButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    if (client->GetMerchantItemsPage() != 1)
        RequestBuyItems(1);
}

void MerchantWindow::HandleLastPageButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    if (client->GetMerchantItemsPage() != client->GetMerchantItemsPageCount())
        RequestBuyItems(client->GetMerchantItemsPageCount());
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
