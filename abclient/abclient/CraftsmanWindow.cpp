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

#include "CraftsmanWindow.h"
#include "ItemsCache.h"
#include "ItemStatsUIElement.h"
#include "PriceUIElement.h"
#include "SkillManager.h"
#include <abshared/Items.h>
#include <abshared/Attributes.h>

CraftsmanWindow::CraftsmanWindow(Context* context) :
    DialogWindow(context)
{
    SetName(CraftsmanWindow::GetTypeNameStatic());

    DisableLayoutUpdate();
    SetMinSize({ 438, 461 });
    LoadLayout("UI/CraftsmanWindow.xml");
    CreateUI();
    SetResizable(true);
    Center();

    SetStyleAuto();
    EnableLayoutUpdate();
    UpdateLayout();

    SubscribeEvents();
    Clear();
}

CraftsmanWindow::~CraftsmanWindow()
{ }

void CraftsmanWindow::CreateUI()
{
    itemTypesList_ = GetChildStaticCast<DropDownList>("ItemTypeList", true);
    itemTypesList_->SetResizePopup(true);
    SubscribeToEvent(itemTypesList_, E_ITEMSELECTED, URHO3D_HANDLER(CraftsmanWindow, HandleItemTypeSelected));
    searchNameEdit_ = GetChildStaticCast<LineEdit>("SearchTextBox", true);
    SubscribeToEvent(searchNameEdit_, E_TEXTFINISHED, URHO3D_HANDLER(CraftsmanWindow, HandleSearchItemEditTextFinished));
    auto* searchButton = GetChildStaticCast<Button>("SearchButton", true);
    SubscribeToEvent(searchButton, E_RELEASED, URHO3D_HANDLER(CraftsmanWindow, HandleSearchButtonClicked));

    pagingContainer_ = GetChild("PageContainer", true);
    currentPage_ = pagingContainer_->GetChildStaticCast<Text>("CurrentPageText", true);
    auto* firstButton = pagingContainer_->GetChildStaticCast<Button>("FirstPageButton", true);
    SubscribeToEvent(firstButton, E_RELEASED, URHO3D_HANDLER(CraftsmanWindow, HandleFirstPageButtonClicked));
    auto* prevButton = pagingContainer_->GetChildStaticCast<Button>("PrevPageButton", true);
    SubscribeToEvent(prevButton, E_RELEASED, URHO3D_HANDLER(CraftsmanWindow, HandlePrevPageButtonClicked));
    auto* nextButton = pagingContainer_->GetChildStaticCast<Button>("NextPageButton", true);
    SubscribeToEvent(nextButton, E_RELEASED, URHO3D_HANDLER(CraftsmanWindow, HandleNextPageButtonClicked));
    auto* lastButton = pagingContainer_->GetChildStaticCast<Button>("LastPageButton", true);
    SubscribeToEvent(lastButton, E_RELEASED, URHO3D_HANDLER(CraftsmanWindow, HandleLastPageButtonClicked));

    auto* listContainer = GetChild("ListContainer", true);
    listContainer->SetLayoutMode(LM_VERTICAL);
    items_ = listContainer->CreateChild<ListView>("ItemsListView");
    items_->SetStyleAuto();
    items_->SetHighlightMode(HM_ALWAYS);
    SubscribeToEvent(items_, E_ITEMSELECTED, URHO3D_HANDLER(CraftsmanWindow, HandleItemSelected));

    attribDropdown_ = GetChildStaticCast<DropDownList>("AttribDropdown", true);
    attribDropdown_->SetVisible(false);
}

void CraftsmanWindow::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    Button* doitButton = GetChildStaticCast<Button>("DoItButton", true);
    SubscribeToEvent(doitButton, E_RELEASED, URHO3D_HANDLER(CraftsmanWindow, HandleDoItClicked));
    Button* closeButton = GetChildStaticCast<Button>("GoodByeButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(CraftsmanWindow, HandleGoodByeClicked));
    SubscribeToEvent(Events::E_CRAFTSMAN_ITEMS, URHO3D_HANDLER(CraftsmanWindow, HandleCraftsmanItems));
}

void CraftsmanWindow::UpdateList()
{
    PopulateItemTypes();
    uint32_t oldSel = 0;
    auto sel = items_->GetSelectedItem();
    if (sel)
        oldSel = sel->GetVar("Index").GetUInt();

    const auto scrollPos = items_->GetViewPosition();
    items_->DisableLayoutUpdate();
    items_->RemoveAllItems();

    unsigned index = 0;
    unsigned selIndex = M_MAX_UNSIGNED;

    auto* client = GetSubsystem<FwClient>();
    String pagingText;
    pagingText.AppendWithFormat("%d/%d", client->GetMerchantItemsPage(), client->GetMerchantItemsPageCount());
    currentPage_->SetText(pagingText);
    const auto& items = client->GetMerchantItems();
    for (const auto& item : items)
    {
        CreateItem(item);
        if (oldSel != 0 && item.index == oldSel)
            selIndex = index;
        ++index;
    }
    if (selIndex != M_MAX_UNSIGNED)
        items_->SetSelection(selIndex);
    items_->EnableLayoutUpdate();
    items_->SetViewPosition(scrollPos);
    pagingContainer_->SetVisible(client->GetMerchantItemsPageCount() > 1);
}

void CraftsmanWindow::RequestList(uint32_t page, bool keepScrollPos)
{
    auto* client = GetSubsystem<FwClient>();
    const String& search = searchNameEdit_->GetText();
    if (!keepScrollPos)
        items_->SetViewPosition({ 0, 0 });
    client->RequestCrafsmanItems(npcId_, static_cast<uint16_t>(GetSelectedItemType()), search, page);
}

AB::Entities::ItemType CraftsmanWindow::GetSelectedItemType() const
{
    auto* item = itemTypesList_->GetSelectedItem();
    if (!item)
        return AB::Entities::ItemType::Unknown;

    unsigned type = item->GetVar("Type").GetUInt();
    return static_cast<AB::Entities::ItemType>(type);
}

uint32_t CraftsmanWindow::GetSelectedItemIndex() const
{
    auto* item = items_->GetSelectedItem();
    if (!item)
        return 0;

    return item->GetVar("Index").GetUInt();
}

UISelectable* CraftsmanWindow::CreateItem(const ConcreteItem& iItem)
{
    ListView& container = *items_;
    auto* cache = GetSubsystem<ResourceCache>();
    auto* itemsCache = GetSubsystem<ItemsCache>();
    auto item = itemsCache->Get(iItem.index);
    if (!item)
        return nullptr;

    UISelectable* uiS = container.CreateChild<UISelectable>("Item" + String(iItem.index));
    uiS->SetLayoutSpacing(4);
    uiS->SetLayoutMode(LM_HORIZONTAL);
    uiS->SetVar("ID", iItem.id);
    uiS->SetVar("Pos", iItem.pos);
    uiS->SetVar("Count", iItem.count);
    uiS->SetVar("Flags", iItem.flags);
    uiS->SetVar("Index", iItem.index);
    uiS->SetVar("Type", static_cast<unsigned>(iItem.type));

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

    UIElement* priceContainer = uiS->CreateChild<UIElement>("Price");
    priceContainer->SetInternal(true);

    auto setValues = [&](int number)
    {
        auto indexIt = iItem.stats.Find(static_cast<Game::ItemStatIndex>(static_cast<int>(Game::ItemStatIndex::Material1Index) + ((number - 1) * 2)));
        if (indexIt == iItem.stats.End())
            return;
        auto countIt = iItem.stats.Find(static_cast<Game::ItemStatIndex>(static_cast<int>(Game::ItemStatIndex::Material1Count) + ((number - 1) * 2)));
        if (countIt == iItem.stats.End())
            return;

        uint32_t index = indexIt->second_.GetUInt();
        uint32_t count = countIt->second_.GetUInt();
        auto* elem = priceContainer->CreateChild<PriceUIElement>();
        elem->Add(index, count);
    };
    for (int i = 1; i <= 4; ++i)
        setValues(i);

    uiS->SetStyle("CraftsmanListItem");
    Texture2D* texture = cache->GetResource<Texture2D>(item->iconFile_);
    icon->SetTexture(texture);
    icon->SetFullImageRect();
    container.AddItem(uiS);

    return uiS;
}

void CraftsmanWindow::PopulateItemTypes()
{
    auto createDropdownItem = [this](const String& text, AB::Entities::ItemType type)
    {
        Text* result = new Text(context_);
        result->SetText(text);
        result->SetStyle("DropDownItemEnumText");
        result->SetVar("Type", static_cast<unsigned>(type));
        return result;
    };

    AB::Entities::ItemType sel = GetSelectedItemType();
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
            if (type == sel)
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

void CraftsmanWindow::UpdateAttributeList()
{
    attribDropdown_->RemoveAllItems();
    uint32_t itemIndex = GetSelectedItemIndex();
    auto* cache = GetSubsystem<ItemsCache>();
    auto item = cache->Get(itemIndex);
    if (!item)
    {
        attribDropdown_->SetVisible(false);
        return;
    }

    auto createDropdownItem = [this](const AB::Entities::Attribute* attribute)
    {
        Text* result = new Text(context_);
        result->SetText(attribute->name.c_str());
        result->SetStyle("DropDownItemEnumText");
        result->SetVar("Value", static_cast<unsigned>(attribute->index));
        return result;
    };

    const auto attribs = Game::GetPossibleItemAttributes(item->type_);
    auto* sm = GetSubsystem<SkillManager>();
    for (auto a : attribs)
    {
        if (a == Game::Attribute::None)
            continue;
        const auto* attr = sm->GetAttributeByIndex(static_cast<uint32_t>(a));
        attribDropdown_->AddItem(createDropdownItem(attr));
    }
    attribDropdown_->SetVisible(attribDropdown_->GetNumItems() != 0);
}

uint32_t CraftsmanWindow::GetSelectedAttributeIndex() const
{
    auto* item = attribDropdown_->GetSelectedItem();
    if (!item)
        return static_cast<uint32_t>(Game::Attribute::None);

    return item->GetVar("Value").GetUInt();
}

void CraftsmanWindow::HandleCraftsmanItems(StringHash, VariantMap&)
{
    UpdateList();
}

void CraftsmanWindow::HandleDoItClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    uint32_t index = GetSelectedItemIndex();
    if (index == 0)
        return;
    uint32_t attrib = GetSelectedAttributeIndex();
    client->CraftItem(npcId_, index, 1, attrib);
}

void CraftsmanWindow::HandleGoodByeClicked(StringHash, VariantMap&)
{
    npcId_ = 0;
    Close();
}

void CraftsmanWindow::HandlePrevPageButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    if (client->GetMerchantItemsPage() < 2)
        return;
    RequestList(client->GetMerchantItemsPage() - 1);
}

void CraftsmanWindow::HandleNextPageButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    uint32_t page = client->GetMerchantItemsPage() + 1;
    if (page <= client->GetMerchantItemsPageCount())
        RequestList(page);
}

void CraftsmanWindow::HandleFirstPageButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    if (client->GetMerchantItemsPage() != 1)
        RequestList(1);
}

void CraftsmanWindow::HandleLastPageButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    if (client->GetMerchantItemsPage() != client->GetMerchantItemsPageCount())
        RequestList(client->GetMerchantItemsPageCount());
}

void CraftsmanWindow::HandleSearchButtonClicked(StringHash, VariantMap&)
{
    RequestList();
}

void CraftsmanWindow::HandleItemTypeSelected(StringHash, VariantMap&)
{
    RequestList();
}

void CraftsmanWindow::HandleItemSelected(StringHash, VariantMap&)
{
    UpdateAttributeList();
}

void CraftsmanWindow::HandleSearchItemEditTextFinished(StringHash, VariantMap&)
{
    RequestList();
}

void CraftsmanWindow::Initialize(uint32_t npcId)
{
    Clear();
    npcId_ = npcId;
    RequestList();
}

void CraftsmanWindow::Clear()
{
    npcId_ = 0;
    items_->RemoveAllItems();
}
