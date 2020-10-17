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

#include "AccountChestDialog.h"
#include "FwClient.h"
#include "InventoryWindow.h"
#include "Item.h"
#include "ItemUIElement.h"
#include "ItemsCache.h"
#include "NumberInputBox.h"
#include "ShortcutEvents.h"
#include "Shortcuts.h"
#include "WindowManager.h"

AccountChestDialog::AccountChestDialog(Context* context) :
    DialogWindow(context),
    initializted_(false)
{
    SetName(AccountChestDialog::GetTypeNameStatic());

    LoadLayout("UI/AccountChestWindow.xml");
    Center();

    SetStyleAuto();

    SubscribeEvents();
    Clear();
}

AccountChestDialog::~AccountChestDialog()
{
    UnsubscribeFromAllEvents();
}

void AccountChestDialog::HandleChest(StringHash, VariantMap&)
{
    Clear();
    FwClient* net = GetSubsystem<FwClient>();
    const auto& items = net->GetChestItems();
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

void AccountChestDialog::HandleChestItemUpdate(StringHash, VariantMap& eventData)
{
    using namespace Events::ChestItemUpdate;

    FwClient* net = GetSubsystem<FwClient>();
    ItemsCache* itemsCache = GetSubsystem<ItemsCache>();

    uint16_t pos = static_cast<uint16_t>(eventData[P_ITEMPOS].GetUInt());

    const ConcreteItem& iItem = net->GetChestItem(pos);
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

void AccountChestDialog::HandleChestItemRemove(StringHash, VariantMap& eventData)
{
    using namespace Events::ChestItemDelete;
    uint16_t pos = static_cast<uint16_t>(eventData[P_ITEMPOS].GetUInt());
    ConcreteItem item;
    item.type = AB::Entities::ItemType::Unknown;
    item.pos = pos;
    SetItem(nullptr, item);
}

void AccountChestDialog::HandleItemDragMove(StringHash, VariantMap& eventData)
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

void AccountChestDialog::HandleItemDragBegin(StringHash, VariantMap& eventData)
{
    using namespace DragBegin;

    auto* item = reinterpret_cast<ItemUIElement*>(eventData[P_ELEMENT].GetVoidPtr());
    int buttons = eventData[P_BUTTONS].GetInt();
    int lx = eventData[P_X].GetInt();
    int ly = eventData[P_Y].GetInt();
    dragItem_ = item->GetDragItem(buttons, { lx, ly });
    dragItem_->BringToFront();
}

void AccountChestDialog::HandleItemDragCancel(StringHash, VariantMap&)
{
    using namespace DragCancel;
    if (!dragItem_)
        return;
    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragItem_.Get());
    dragItem_.Reset();
}

void AccountChestDialog::HandleItemDragEnd(StringHash, VariantMap& eventData)
{
    using namespace DragEnd;
    if (!dragItem_)
        return;
    uint16_t pos = static_cast<uint16_t>(dragItem_->GetVar("Pos").GetUInt());

    auto* itemsCache = GetSubsystem<ItemsCache>();
    auto item = itemsCache->Get(dragItem_->GetVar("Index").GetUInt());
    if (!item)
        return;

    ConcreteItem ci;
    ci.pos = pos;
    ci.pos = pos;
    ci.type = item->type_;
    ci.place = AB::Entities::StoragePlace::Chest;
    ci.index = dragItem_->GetVar("Index").GetUInt();
    ci.count = dragItem_->GetVar("Count").GetUInt();
    ci.value = static_cast<uint16_t>(dragItem_->GetVar("Value").GetUInt());
    LoadStatsFromString(ci.stats, dragItem_->GetVar("Stats").GetString());

    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();
    if (IsInside({ X, Y }, true))
        DropItem({ X, Y }, ci);
    else
    {
        // If dropping on the players inventory move it there
        WindowManager* wm = GetSubsystem<WindowManager>();
        auto inventory = wm->GetWindow(WINDOW_INVENTORY);
        if (inventory && inventory->IsVisible())
        {
            if (inventory->IsInside({ X, Y }, true))
                static_cast<InventoryWindow*>(inventory.Get())->DropItem({ X, Y }, ci);
        }
    }

    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragItem_.Get());
    dragItem_.Reset();
}

void AccountChestDialog::HandleDepositClicked(StringHash, VariantMap&)
{
    if (inputBox_)
        inputBox_->Close();

    inputBox_ = MakeShared<NumberInputBox>(context_, "Deposit Money");
    auto* client = GetSubsystem<FwClient>();
    uint32_t invMoney = client->GetInventoryMoney();
    uint32_t chestCap = client->GetChestLimit().maxMoney - client->GetChestMoney();
    uint32_t max = std::min(invMoney, chestCap);
    inputBox_->SetMax(static_cast<int>(max));
    inputBox_->SetShowMaxButton(true);
    inputBox_->SetMin(1);
    inputBox_->SelectAll();
    SubscribeToEvent(inputBox_, E_NUMBERINPUTBOXDONE, URHO3D_HANDLER(AccountChestDialog, HandleDepositDone));
    SubscribeToEvent(inputBox_, E_DIALOGCLOSE, URHO3D_HANDLER(AccountChestDialog, HandleDialogClosed));
}

void AccountChestDialog::HandleWithdrawClicked(StringHash, VariantMap&)
{
    if (inputBox_)
        inputBox_->Close();

    inputBox_ = MakeShared<NumberInputBox>(context_, "Withdraw Money");
    auto* client = GetSubsystem<FwClient>();
    int chestMoney = static_cast<int>(client->GetChestMoney());
    int invCap = static_cast<int>(client->GetInventoryLimit().maxMoney) - static_cast<int>(client->GetInventoryMoney());
    int max = std::min(chestMoney, invCap);
    inputBox_->SetMax(max);
    inputBox_->SetShowMaxButton(true);
    inputBox_->SetMin(1);
    inputBox_->SelectAll();
    SubscribeToEvent(inputBox_, E_NUMBERINPUTBOXDONE, URHO3D_HANDLER(AccountChestDialog, HandleWithdrawDone));
    SubscribeToEvent(inputBox_, E_DIALOGCLOSE, URHO3D_HANDLER(AccountChestDialog, HandleDialogClosed));
}

void AccountChestDialog::HandleWithdrawDone(StringHash, VariantMap& eventData)
{
    using namespace NumberInputBoxDone;
    if (!eventData[P_OK].GetBool())
        return;

    auto* client = GetSubsystem<FwClient>();
    client->WithdrawMoney(eventData[P_VALUE].GetUInt());
}

void AccountChestDialog::HandleDepositDone(StringHash, VariantMap& eventData)
{
    using namespace NumberInputBoxDone;
    if (!eventData[P_OK].GetBool())
        return;

    auto* client = GetSubsystem<FwClient>();
    client->DepositMoney(eventData[P_VALUE].GetUInt());
}

void AccountChestDialog::HandleDialogClosed(StringHash, VariantMap&)
{
    if (inputBox_)
        inputBox_.Reset();
}

uint16_t AccountChestDialog::GetItemPosFromClientPos(const IntVector2& clientPos)
{
    auto* container = GetChild("Container", true);
    auto* moneyRow = container->GetChild("MoneyRow", true);
    IntVector2 item;
    item.y_ = ((clientPos.y_ - (container->GetPosition().y_ + moneyRow->GetPosition().y_ + moneyRow->GetHeight())) / CHEST_ITEM_SIZE_Y);
    item.x_ = (clientPos.x_ / CHEST_ITEM_SIZE_X) + 1;
    if (item.y_ < 0 || item.x_ < 0)
        return 0;
    return static_cast<uint16_t>((item.y_ * CHEST_COLS_PER_ROW) + item.x_);
}

void AccountChestDialog::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    SubscribeToEvent(Events::E_CHEST, URHO3D_HANDLER(AccountChestDialog, HandleChest));
    SubscribeToEvent(Events::E_CHESTITEMUPDATE, URHO3D_HANDLER(AccountChestDialog, HandleChestItemUpdate));
    SubscribeToEvent(Events::E_CHESTITEMDELETE, URHO3D_HANDLER(AccountChestDialog, HandleChestItemRemove));
    Button* depositButton = GetChildStaticCast<Button>("DespositButton", true);
    SubscribeToEvent(depositButton, E_RELEASED, URHO3D_HANDLER(AccountChestDialog, HandleDepositClicked));
    Button* withdrawButton = GetChildStaticCast<Button>("WithdrawButton", true);
    SubscribeToEvent(withdrawButton, E_RELEASED, URHO3D_HANDLER(AccountChestDialog, HandleWithdrawClicked));
}

void AccountChestDialog::Initialize(uint32_t)
{
    if (!initializted_)
    {
        GetSubsystem<FwClient>()->UpdateChest();
        initializted_ = true;
    }
}

void AccountChestDialog::HandleItemCountDone(StringHash, VariantMap& eventData)
{
    using namespace NumberInputBoxDone;
    if (!eventData[P_OK].GetBool())
        return;

    uint16_t currentPos = static_cast<uint16_t>(inputBox_->GetVar("CurrentPos").GetUInt());
    AB::Entities::StoragePlace currentPlace = static_cast<AB::Entities::StoragePlace>(inputBox_->GetVar("CurrentPlace").GetUInt());
    uint16_t newPos = static_cast<uint16_t>(inputBox_->GetVar("Pos").GetUInt());
    auto* client = GetSubsystem<FwClient>();
    client->SetItemPos(currentPlace, currentPos,
        AB::Entities::StoragePlace::Chest, newPos, eventData[P_VALUE].GetUInt());
}

void AccountChestDialog::DropItem(const IntVector2& screenPos, const ConcreteItem& ci)
{
    if (!IsInside(screenPos, true))
        return;

    IntVector2 clientPos = screenPos - GetScreenPosition();
    uint16_t itemPos = GetItemPosFromClientPos(clientPos);
    if (itemPos == 0)
        return;

    auto* sc = GetSubsystem<Shortcuts>();
    if (ci.count == 1 || !sc->IsTriggered(Events::E_SC_SPLITSTACK))
    {
        auto* client = GetSubsystem<FwClient>();
        client->SetItemPos(ci.place, ci.pos,
            AB::Entities::StoragePlace::Chest, itemPos, ci.count);
        return;
    }

    if (inputBox_)
        inputBox_->Close();

    inputBox_ = MakeShared<NumberInputBox>(context_, "How many?");
    inputBox_->SetMax(static_cast<int>(ci.count));
    inputBox_->SetVar("Pos", itemPos);
    inputBox_->SetVar("CurrentPlace", static_cast<unsigned>(ci.place));
    inputBox_->SetVar("CurrentPos", static_cast<unsigned>(ci.pos));
    inputBox_->SetShowMaxButton(true);
    inputBox_->SetMin(1);
    inputBox_->SetValue(1);
    inputBox_->SelectAll();
    SubscribeToEvent(inputBox_, E_NUMBERINPUTBOXDONE, URHO3D_HANDLER(AccountChestDialog, HandleItemCountDone));
    SubscribeToEvent(inputBox_, E_DIALOGCLOSE, URHO3D_HANDLER(AccountChestDialog, HandleDialogClosed));
}

BorderImage* AccountChestDialog::GetItemContainer(uint16_t pos)
{
    // pos is 1-based
    unsigned rowIndex = (pos - 1) / CHEST_COLS_PER_ROW;
    UIElement* container = GetChild("Container", true);
    String name("ItemRow" + String(rowIndex + 1));
    UIElement* row = container->GetChild(name, true);
    if (!row)
        return nullptr;
    unsigned index = pos - (rowIndex * CHEST_COLS_PER_ROW) - 1;
    BorderImage* result = row->GetChildStaticCast<BorderImage>(index);
    return result;
}

void AccountChestDialog::SetItem(Item* item, const ConcreteItem& iItem)
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
    itemElem->SetStats(iItem.stats);
    SubscribeToEvent(itemElem, E_DRAGMOVE, URHO3D_HANDLER(AccountChestDialog, HandleItemDragMove));
    SubscribeToEvent(itemElem, E_DRAGBEGIN, URHO3D_HANDLER(AccountChestDialog, HandleItemDragBegin));
    SubscribeToEvent(itemElem, E_DRAGCANCEL, URHO3D_HANDLER(AccountChestDialog, HandleItemDragCancel));
    SubscribeToEvent(itemElem, E_DRAGEND, URHO3D_HANDLER(AccountChestDialog, HandleItemDragEnd));
}

void AccountChestDialog::Clear()
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
