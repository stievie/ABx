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
    tabgroup_->SetSelectedIndex(0);
}

void MerchantWindow::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    Button* closeButton = GetChildStaticCast<Button>("GoodByeButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleGoodByeClicked));
    SubscribeToEvent(Events::E_MERCHANT_ITEMS, URHO3D_HANDLER(MerchantWindow, HandleMerchantItems));
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
    auto* client = GetSubsystem<FwClient>();
    auto* itemsCache = GetSubsystem<ItemsCache>();
    const auto& items = client->GetInventoryItems();
    for (const auto& ci : items)
    {
        auto item = itemsCache->Get(ci.index);
        if (!item->tradeAble_)
            continue;
        CreateItem(sellItems_, ci);
    }
    sellItems_->UpdateLayout();
}

UISelectable* MerchantWindow::CreateItem(ListView* container, const ConcreteItem& iItem)
{
    auto* itemsCache = GetSubsystem<ItemsCache>();
    auto item = itemsCache->Get(iItem.index);
    if (!item)
        return nullptr;

    UISelectable* uiS = container->CreateChild<UISelectable>();
    uiS->SetStyleAuto();
    uiS->SetMinHeight(44);
    uiS->SetMaxHeight(44);
    uiS->SetLayoutBorder({ 4, 4, 4, 4 });
    uiS->SetLayoutMode(LM_HORIZONTAL);

    ItemUIElement* itemElem = uiS->CreateChild<ItemUIElement>("ItemElememt");
    itemElem->SetMinSize({ 40, 40 });
    itemElem->SetMaxSize({ 40, 40 });
    itemElem->SetName(item->name_);
    itemElem->SetIcon(item->iconFile_);
    itemElem->SetPos(iItem.pos);
    itemElem->SetIndex(iItem.index);
    itemElem->SetCount(iItem.count);
    itemElem->SetValue(iItem.value);
    itemElem->SetStats(SaveStatsToString(iItem.stats));

    Text* nameElem = uiS->CreateChild<Text>();
    nameElem->SetText(item->name_);
    Text* valueElem = uiS->CreateChild<Text>();
    valueElem->SetText(String(iItem.value) + " D");

    uiS->UpdateLayout();
    container->AddItem(uiS);

    SubscribeToEvent(uiS, E_RELEASED, URHO3D_HANDLER(MerchantWindow, HandleItemClicked));

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

void MerchantWindow::HandleGoodByeClicked(StringHash, VariantMap&)
{
    Close();
}

void MerchantWindow::HandleItemClicked(StringHash, VariantMap&)
{
}

void MerchantWindow::Initialize()
{
    Clear();
    GetSubsystem<FwClient>()->GetMerchantItems();
}

void MerchantWindow::Clear()
{
}
