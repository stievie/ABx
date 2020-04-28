/**
 * Copyright 2017-2020 Stefan Ascher
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
#include "InventoryWindow.h"
#include "Shortcuts.h"
#include "FwClient.h"
#include "ItemsCache.h"
#include "Item.h"
#include "WindowManager.h"
#include "AccountChestDialog.h"
#include "TradeDialog.h"
#include "LevelManager.h"
#include "WorldLevel.h"
#include "NumberInputBox.h"
#include "ItemUIElement.h"

void InventoryWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<InventoryWindow>();
}

InventoryWindow::InventoryWindow(Context* context) :
    Window(context),
    initializted_(false)
{
    SetName("InventoryWindow");

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/InventoryWindow.xml");
    LoadXML(file->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(true);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));
    SetBringToFront(true);
    SetBringToBack(true);

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = GetChildStaticCast<Text>("CaptionText", true);
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLEINVENTORYWINDOW, "Inventory", true));

    Text* moneyText = GetChildStaticCast<Text>("MoneyText", true);
    moneyText->SetText("0 Drachma");
    SetSize(260, 480);
    SetPosition(10, 30);
    SetVisible(true);

    SetStyleAuto();

    itemPopup_ = MakeShared<Menu>(context_);
    itemPopup_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    itemPopup_->SetStyleAuto();
    Window* popup = itemPopup_->CreateChild<Window>();
    popup->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    popup->SetStyle("MenuPopupWindow");
    popup->SetLayout(LM_VERTICAL, 1);
    itemPopup_->SetPopup(popup);

    int width = 0;
    int height = 0;

    {
        // Destroy
        Menu* item = popup->CreateChild<Menu>();
        item->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
        item->SetStyleAuto();
        Text* menuText = item->CreateChild<Text>();
        menuText->SetText("Destroy");
        menuText->SetStyle("EditorMenuText");
        item->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
        item->SetMinSize(menuText->GetSize() + IntVector2(4, 4));
        item->SetSize(item->GetMinSize());
        if (item->GetWidth() > width)
            width = item->GetWidth();
        if (item->GetHeight() > height)
            height = item->GetHeight();
        SubscribeToEvent(item, E_MENUSELECTED, URHO3D_HANDLER(InventoryWindow, HandleItemDestroySelected));
    }
    {
        // Drop
        Menu* item = popup->CreateChild<Menu>();
        item->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
        item->SetStyleAuto();
        Text* menuText = item->CreateChild<Text>();
        menuText->SetText("Drop");
        menuText->SetStyle("EditorMenuText");
        item->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
        item->SetMinSize(menuText->GetSize() + IntVector2(4, 4));
        item->SetSize(item->GetMinSize());
        if (item->GetWidth() > width)
            width = item->GetWidth();
        if (item->GetHeight() > height)
            height = item->GetHeight();
        SubscribeToEvent(item, E_MENUSELECTED, URHO3D_HANDLER(InventoryWindow, HandleItemDropSelected));
    }

    popup->SetMinSize(IntVector2(width, height));
    popup->SetSize(popup->GetMinSize());
    itemPopup_->SetSize(popup->GetSize());

    SubscribeEvents();
}

InventoryWindow::~InventoryWindow()
{
    UnsubscribeFromAllEvents();
}

void InventoryWindow::GetInventory()
{
    if (!initializted_)
    {
        FwClient* net = GetSubsystem<FwClient>();
        net->UpdateInventory();
        initializted_ = true;
    }
}

void InventoryWindow::Clear()
{
    Text* moneyText = GetChildStaticCast<Text>("MoneyText", true);
    moneyText->SetText("0 Drachma");
    money_ = 0;
    uint16_t pos = 1;
    while (auto* cont = GetItemContainer(pos))
    {
        cont->RemoveAllChildren();
        ++pos;
    }
    initializted_ = false;
}

void InventoryWindow::SubscribeEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(InventoryWindow, HandleCloseClicked));
    SubscribeToEvent(Events::E_INVENTORY, URHO3D_HANDLER(InventoryWindow, HandleInventory));
    SubscribeToEvent(Events::E_INVENTORYITEMUPDATE, URHO3D_HANDLER(InventoryWindow, HandleInventoryItemUpdate));
    SubscribeToEvent(Events::E_INVENTORYITEMDELETE, URHO3D_HANDLER(InventoryWindow, HandleInventoryItemRemove));
}

void InventoryWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void InventoryWindow::SetItem(Item* item, const ConcreteItem& iItem)
{
    BorderImage* container = GetItemContainer(iItem.pos);
    if (!container)
        return;

    container->RemoveAllChildren();
    if (item == nullptr)
        // The item was removed
        return;

    ItemUIElement* itemElem = container->CreateChild<ItemUIElement>("ItemElememt");
    itemElem->SetPosition(4, 4);
    itemElem->SetSize(container->GetSize() - IntVector2(8, 8));
    itemElem->SetMinSize(itemElem->GetSize());
    itemElem->SetName(item->name_);
    itemElem->SetIcon(item->iconFile_);
    itemElem->SetPos(iItem.pos);
    itemElem->SetIndex(iItem.index);
    itemElem->SetCount(iItem.count);
    itemElem->SetValue(iItem.value);
    itemElem->SetStats(SaveStatsToString(iItem.stats));

    SubscribeToEvent(itemElem, E_CLICKEND, URHO3D_HANDLER(InventoryWindow, HandleItemClicked));
    SubscribeToEvent(itemElem, E_DRAGMOVE, URHO3D_HANDLER(InventoryWindow, HandleItemDragMove));
    SubscribeToEvent(itemElem, E_DRAGBEGIN, URHO3D_HANDLER(InventoryWindow, HandleItemDragBegin));
    SubscribeToEvent(itemElem, E_DRAGCANCEL, URHO3D_HANDLER(InventoryWindow, HandleItemDragCancel));
    SubscribeToEvent(itemElem, E_DRAGEND, URHO3D_HANDLER(InventoryWindow, HandleItemDragEnd));
}

void InventoryWindow::HandleInventory(StringHash, VariantMap&)
{
    Clear();
    FwClient* net = GetSubsystem<FwClient>();
    const auto& items = net->GetInventoryItems();
    ItemsCache* itemsCache = GetSubsystem<ItemsCache>();

    Text* moneyText = GetChildStaticCast<Text>("MoneyText", true);

    for (const auto& item : items)
    {
        if (item.type == AB::Entities::ItemType::Money)
        {
            moneyText->SetText(FormatMoney(item.count) + " Drachma");
            money_ = item.count;
            continue;
        }

        Item* i = itemsCache->Get(item.index);
        if (!i)
            continue;
        SetItem(i, item);
    }
    SetStyleAuto();
}

void InventoryWindow::HandleInventoryItemUpdate(StringHash, VariantMap& eventData)
{
    using namespace Events::InventoryItemUpdate;

    FwClient* net = GetSubsystem<FwClient>();
    ItemsCache* itemsCache = GetSubsystem<ItemsCache>();

    uint16_t pos = static_cast<uint16_t>(eventData[P_ITEMPOS].GetUInt());

    const ConcreteItem& iItem = net->GetInventoryItem(pos);
    if (iItem.type == AB::Entities::ItemType::Unknown)
        return;

    if (iItem.type == AB::Entities::ItemType::Money)
    {
        Text* moneyText = GetChildStaticCast<Text>("MoneyText", true);
        moneyText->SetText(FormatMoney(iItem.count) + " Drachma");
        money_ = iItem.count;
        return;
    }
    Item* i = itemsCache->Get(iItem.index);
    if (!i)
        return;
    SetItem(i, iItem);
}

void InventoryWindow::HandleInventoryItemRemove(StringHash, VariantMap& eventData)
{
    using namespace Events::InventoryItemDelete;
    uint16_t pos = static_cast<uint16_t>(eventData[P_ITEMPOS].GetUInt());
    ConcreteItem item;
    item.type = AB::Entities::ItemType::Unknown;
    item.pos = pos;
    SetItem(nullptr, item);
}

void InventoryWindow::HandleItemClicked(StringHash, VariantMap& eventData)
{
    using namespace ClickEnd;
    MouseButton button = static_cast<MouseButton>(eventData[P_BUTTON].GetUInt());
    ItemUIElement* elem = dynamic_cast<ItemUIElement*>(eventData[P_ELEMENT].GetPtr());
    if (!elem)
        return;
    if (button == MOUSEB_RIGHT)
    {
        int x = eventData[P_X].GetInt();
        int y = eventData[P_Y].GetInt();
        itemPopup_->SetPosition(x, y);
        itemPopup_->ShowPopup(true);
        itemPopup_->SetVar("ItemPos", elem->pos_);
    }
}

void InventoryWindow::HandleItemDestroySelected(StringHash, VariantMap& eventData)
{
    itemPopup_->ShowPopup(false);
    using namespace MenuSelected;
    Menu* sender = dynamic_cast<Menu*>(eventData[P_ELEMENT].GetPtr());
    if (!sender)
        return;
    unsigned pos = itemPopup_->GetVar("ItemPos").GetUInt();
    if (pos == 0)
        return;
    FwClient* cli = GetSubsystem<FwClient>();
    cli->InventoryDestroyItem(static_cast<uint16_t>(pos));
}

void InventoryWindow::HandleItemDropSelected(StringHash, VariantMap& eventData)
{
    itemPopup_->ShowPopup(false);
    using namespace MenuSelected;
    Menu* sender = dynamic_cast<Menu*>(eventData[P_ELEMENT].GetPtr());
    if (!sender)
        return;
    unsigned pos = itemPopup_->GetVar("ItemPos").GetUInt();
    if (pos == 0)
        return;
    FwClient* cli = GetSubsystem<FwClient>();
    cli->InventoryDropItem(static_cast<uint16_t>(pos));
}

void InventoryWindow::HandleItemDragMove(StringHash, VariantMap& eventData)
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

void InventoryWindow::HandleItemDragBegin(StringHash, VariantMap& eventData)
{
    using namespace DragBegin;
    auto* item = reinterpret_cast<ItemUIElement*>(eventData[P_ELEMENT].GetVoidPtr());
    int buttons = eventData[P_BUTTONS].GetInt();
    int lx = eventData[P_X].GetInt();
    int ly = eventData[P_Y].GetInt();
    dragItem_ = item->GetDragItem(buttons, { lx, ly });
    dragItem_->BringToFront();
}

void InventoryWindow::HandleItemDragCancel(StringHash, VariantMap&)
{
    using namespace DragCancel;
    if (!dragItem_)
        return;
    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragItem_.Get());
    dragItem_.Reset();
}

void InventoryWindow::HandleItemDragEnd(StringHash, VariantMap& eventData)
{
    using namespace DragEnd;
    if (!dragItem_)
        return;
    uint16_t pos = static_cast<uint16_t>(dragItem_->GetVar("Pos").GetUInt());
    WindowManager* wm = GetSubsystem<WindowManager>();
    auto dialog = wm->GetDialog(AB::DialogAccountChest);
    AccountChestDialog* chest = dynamic_cast<AccountChestDialog*>(dialog.Get());
    TradeDialog* tradeDialog = nullptr;
    auto* lm = GetSubsystem<LevelManager>();
    auto* level = lm->GetCurrentLevel<WorldLevel>();
    if (level)
        tradeDialog = level->GetTradeDialog();

    auto* itemsCache = GetSubsystem<ItemsCache>();
    auto item = itemsCache->Get(dragItem_->GetVar("Index").GetUInt());
    if (!item)
        return;

    ConcreteItem ci;
    ci.pos = pos;
    ci.pos = pos;
    ci.type = item->type_;
    ci.place = AB::Entities::StoragePlace::Inventory;
    ci.index = dragItem_->GetVar("Index").GetUInt();
    ci.count = dragItem_->GetVar("Count").GetUInt();
    ci.value = static_cast<uint16_t>(dragItem_->GetVar("Value").GetUInt());
    LoadStatsFromString(ci.stats, dragItem_->GetVar("Stats").GetString());

    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();
    if (IsInside({ X, Y }, true))
        DropItem({ X, Y }, ci);
    else if (chest && chest->IsInside({ X, Y }, true))
    {
        // If dropping on the account chest it is store in the chest
        chest->DropItem({ X, Y }, ci);
    }
    else if (tradeDialog && tradeDialog->IsInside({ X, Y }, true))
    {
        if (item->tradeAble_)
            tradeDialog->DropItem({ X, Y }, std::move(ci));
    }

    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragItem_.Get());
    dragItem_.Reset();
}

bool InventoryWindow::DropItem(const IntVector2& screenPos, const ConcreteItem& ci)
{
    if (!IsInside(screenPos, true))
        return false;

    IntVector2 clientPos = screenPos - GetScreenPosition();
    uint16_t itemPos = GetItemPosFromClientPos(clientPos);
    if (itemPos == 0)
        return false;

    auto* client = GetSubsystem<FwClient>();
    client->SetItemPos(ci.place, ci.pos,
        AB::Entities::StoragePlace::Inventory, itemPos, ci.count);

    return true;
}

uint16_t InventoryWindow::GetItemPosFromClientPos(const IntVector2& clientPos)
{
    auto* container = GetChild("Container", true);
    auto* moneyRow = container->GetChild("MoneyRow", true);
    IntVector2 item;
    item.y_ = ((clientPos.y_ - (container->GetPosition().y_ + moneyRow->GetPosition().y_ + moneyRow->GetHeight())) / INVENTORY_ITEM_SIZE_Y);
    item.x_ = (clientPos.x_ / INVENTORY_ITEM_SIZE_X) + 1;
    if (item.y_ < 0 || item.x_ < 0)
        return 0;
    return static_cast<uint16_t>((item.y_ * INVENTORY_COLS_PER_ROW) + item.x_);
}

BorderImage* InventoryWindow::GetItemContainer(uint16_t pos)
{
    // pos is 1-based
    unsigned rowIndex = (pos - 1) / INVENTORY_COLS_PER_ROW;
    UIElement* container = GetChild("Container", true);
    String name("ItemRow" + String(rowIndex + 1));
    UIElement* row = container->GetChild(name, true);
    if (!row)
        return nullptr;
    unsigned index = pos - (rowIndex * INVENTORY_COLS_PER_ROW) - 1;
    BorderImage* result = row->GetChildStaticCast<BorderImage>(index);
    return result;
}
