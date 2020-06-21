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
    SubscribeToEvent(tabgroup_, E_TABSELECTED, URHO3D_HANDLER(MerchantWindow, HandleTabSelected));
    tabgroup_->SetEnabled(true);
    tabgroup_->SetSelectedIndex(0);
}

void MerchantWindow::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    Button* sellButton = GetChildStaticCast<Button>("SellButton", true);
    SubscribeToEvent(sellButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleSellClicked));
    Button* closeButton = GetChildStaticCast<Button>("GoodByeButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleGoodByeClicked));
    SubscribeToEvent(Events::E_MERCHANT_ITEMS, URHO3D_HANDLER(MerchantWindow, HandleMerchantItems));
    SubscribeToEvent(Events::E_INVENTORY, URHO3D_HANDLER(MerchantWindow, HandleInventory));
    SubscribeToEvent(Events::E_INVENTORYITEMUPDATE, URHO3D_HANDLER(MerchantWindow, HandleInventoryItemUpdate));
    SubscribeToEvent(Events::E_INVENTORYITEMDELETE, URHO3D_HANDLER(MerchantWindow, HandleInventoryItemRemove));
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

    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/MerchantWindowPageItems.xml");
    sellItems_ = wnd->GetChildStaticCast<ListView>("ItemsListView", true);
    sellItems_->SetStyleAuto();
    page->UpdateLayout();
}

void MerchantWindow::CreatePageBuy(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/MerchantWindowPageItems.xml");
    buyItems_ = wnd->GetChildStaticCast<ListView>("ItemsListView", true);
    buyItems_->SetStyleAuto();
    page->UpdateLayout();
}

void MerchantWindow::LoadWindow(Window* wnd, const String& fileName)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xml = cache->GetResource<XMLFile>(fileName);
    wnd->LoadXML(xml->GetRoot());
    // It seems this isn't loaded from the XML file
    wnd->SetLayoutMode(LM_VERTICAL);
    wnd->SetLayoutBorder(IntRect(4, 4, 4, 4));
    wnd->SetPivot(0, 0);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    wnd->SetTexture(tex);
    wnd->SetImageRect(IntRect(48, 0, 64, 16));
    wnd->SetBorder(IntRect(4, 4, 4, 4));
    wnd->SetImageBorder(IntRect(0, 0, 0, 0));
    wnd->SetResizeBorder(IntRect(8, 8, 8, 8));
}

void MerchantWindow::UpdateSellList()
{
    sellItems_->RemoveAllItems();
    auto* client = GetSubsystem<FwClient>();
    auto* itemsCache = GetSubsystem<ItemsCache>();
    const auto& items = client->GetInventoryItems();
    for (const auto& ci : items)
    {
        auto item = itemsCache->Get(ci.index);
        if (item->type_ == AB::Entities::ItemType::Money)
            continue;
        if (!item->tradeAble_)
            continue;
        if (ci.value == 0 || ci.count == 0)
            continue;

        CreateItem(*sellItems_, ci);
    }
    sellItems_->UpdateLayout();
    URHO3D_LOGINFOF("Sell items count %d", sellItems_->GetNumItems());
}

UIElement* MerchantWindow::GetSellItem(uint16_t pos)
{
    for (unsigned i = 0; i < sellItems_->GetNumItems(); ++i)
    {
        auto* item = sellItems_->GetItem(i);
        if (!item)
            continue;
        if (item->GetVar("Pos").GetUInt() == pos)
            return item;
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
    uiS->SetVar("Pos", iItem.pos);
    uiS->SetVar("Count", iItem.count);

    BorderImage* icon = uiS->CreateChild<BorderImage>("Icon");
    icon->SetInternal(true);

    UIElement* textContainer = uiS->CreateChild<UIElement>();
    textContainer->SetInternal(true);

    Text* nameElem = textContainer->CreateChild<Text>("Name");
    nameElem->SetText(item->name_);
    nameElem->SetInternal(true);
    Text* countElem = textContainer->CreateChild<Text>("Count");
    if (iItem.count > 1)
        countElem->SetText(String(iItem.count) + " x");
    countElem->SetInternal(true);
    Text* valueElem = textContainer->CreateChild<Text>("Value");
    valueElem->SetText(FormatMoney(iItem.value * iItem.count) + " Drachma");
    valueElem->SetInternal(true);

    uiS->SetStyle("MerchantListItem");
    Texture2D* texture = cache->GetResource<Texture2D>(item->iconFile_);
    icon->SetTexture(texture);
    icon->SetFullImageRect();
    container.AddItem(uiS);

    return uiS;
}

void MerchantWindow::HandleMerchantItems(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    const auto& items = client->GetMerchantItems();
    for (const auto& item : items)
    {
        (void)item;
    }
}

void MerchantWindow::HandleSellClicked(StringHash, VariantMap&)
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
        URHO3D_LOGINFOF("Selling %d, count %d", pos, count);
        client->SellItem(npcId_, pos, count);
    }
    else
    {
        // Buy
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
    using namespace TabSelected;
    int index = eventData[P_INDEX].GetInt();
    if (index == 0)
    {
        Text* txt = doItButton_->GetChildStaticCast<Text>("DoItButtonText", true);
        txt->SetText("Sell");
    }
    else
    {
        Text* txt = doItButton_->GetChildStaticCast<Text>("DoItButtonText", true);
        txt->SetText("Buy");
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
        countText_->SetVisible(false);
        return;
    }

    unsigned count = item->GetVar("Count").GetUInt();
    if (count > 1)
    {
        countText_->SetVisible(true);
        countSpinner_->SetMin(1);
        countSpinner_->SetMax(count);
        countSpinner_->SetValue(count);
    }
    else
    {
        countText_->SetVisible(false);
    }
    URHO3D_LOGINFOF("Sell item selected %d, count %d", index, count);
}

void MerchantWindow::Initialize(uint32_t npcId)
{
    Clear();
    npcId_ = npcId;
    auto* client = GetSubsystem<FwClient>();
    client->UpdateInventory();

    client->GetMerchantItems();
}

void MerchantWindow::Clear()
{
    npcId_ = 0;
    sellItems_->RemoveAllItems();
    buyItems_->RemoveAllItems();
}
