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
#include "AccountChestDialog.h"
#include "FwClient.h"
#include "Item.h"
#include "ItemsCache.h"
#include "WindowManager.h"
#include "InventoryWindow.h"
#include "NumberInputBox.h"

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
        if (item.type == AB::Entities::ItemTypeMoney)
        {
            moneyText->SetText(FormatMoney(item.count) + " Drachma");
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

    const InventoryItem& iItem = net->GetChestItem(pos);
    if (iItem.type == AB::Entities::ItemTypeUnknown)
        return;

    if (iItem.type == AB::Entities::ItemTypeMoney)
    {
        Text* moneyText = GetChildStaticCast<Text>("MoneyText", true);
        moneyText->SetText(FormatMoney(iItem.count) + " Drachma");
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
    InventoryItem item;
    item.type = AB::Entities::ItemTypeUnknown;
    item.pos = pos;
    SetItem(nullptr, item);
}

void AccountChestDialog::HandleItemClicked(StringHash, VariantMap& eventData)
{
    // TODO: What???
    (void)eventData;
}

void AccountChestDialog::HandleItemDragMove(StringHash, VariantMap& eventData)
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

void AccountChestDialog::HandleItemDragBegin(StringHash, VariantMap& eventData)
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

    int lx = eventData[P_X].GetInt();
    int ly = eventData[P_Y].GetInt();
    dragItem_->SetPosition(IntVector2(lx, ly) - dragItem_->GetSize() / 2);

    int buttons = eventData[P_BUTTONS].GetInt();
    item->SetVar("BUTTONS", buttons);
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
    uint16_t pos = static_cast<uint16_t>(dragItem_->GetVar("POS").GetUInt());

    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();
    if (IsInside({ X, Y }, true))
        DropItem({ X, Y }, AB::Entities::StoragePlaceChest, pos);
    else
    {
        // If dropping on the players inventory move it there
        WindowManager* wm = GetSubsystem<WindowManager>();
        auto inv = wm->GetWindow(WINDOW_INVENTORY);
        if (inv && inv->IsVisible())
        {
            if (inv->IsInside({ X, Y }, true))
                static_cast<InventoryWindow*>(inv.Get())->DropItem({ X, Y }, AB::Entities::StoragePlaceChest, pos);
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
    SubscribeToEvent(inputBox_, E_NUMBERINPUTBOXDONE, URHO3D_HANDLER(AccountChestDialog, HandleDepositDone));
    SubscribeToEvent(inputBox_, E_DIALOGCLOSE, URHO3D_HANDLER(AccountChestDialog, HandleDialogClosed));
}

void AccountChestDialog::HandleWithdrawClicked(StringHash, VariantMap&)
{
    if (inputBox_)
        inputBox_->Close();

    inputBox_ = MakeShared<NumberInputBox>(context_, "Withdraw Money");
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
//    URHO3D_LOGINFOF("X = %d, Y = %d", item.x_, item.y_);
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

void AccountChestDialog::Initialize()
{
    if (!initializted_)
    {
        GetSubsystem<FwClient>()->UpdateChest();
        initializted_ = true;
    }
}

bool AccountChestDialog::DropItem(const IntVector2& screenPos, AB::Entities::StoragePlace currentPlace, uint16_t currItemPos)
{
    if (!IsInside(screenPos, true))
        return false;

    IntVector2 clientPos = screenPos - GetScreenPosition();
    uint16_t itemPos = GetItemPosFromClientPos(clientPos);
    if (itemPos == 0)
        return false;

    auto* client = GetSubsystem<FwClient>();
    client->SetItemPos(currentPlace, currItemPos,
        AB::Entities::StoragePlaceChest, itemPos);

    return true;
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

void AccountChestDialog::SetItem(Item* item, const InventoryItem& iItem)
{
    BorderImage* container = GetItemContainer(iItem.pos);
    if (!container)
        return;

    container->RemoveAllChildren();
    if (item == nullptr)
        // The item was removed
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    // For ToolTips we need a button
    Button* icon = container->CreateChild<Button>("Icon");
    icon->SetPosition(4, 4);
    icon->SetSize(container->GetSize() - IntVector2(8, 8));
    icon->SetMinSize(icon->GetSize());
    Texture2D* texture = cache->GetResource<Texture2D>(item->iconFile_);
    icon->SetTexture(texture);
    icon->SetFullImageRect();
    icon->SetLayoutMode(LM_FREE);
    icon->SetVar("POS", iItem.pos);
    SubscribeToEvent(icon, E_CLICKEND, URHO3D_HANDLER(AccountChestDialog, HandleItemClicked));
    SubscribeToEvent(icon, E_DRAGMOVE, URHO3D_HANDLER(AccountChestDialog, HandleItemDragMove));
    SubscribeToEvent(icon, E_DRAGBEGIN, URHO3D_HANDLER(AccountChestDialog, HandleItemDragBegin));
    SubscribeToEvent(icon, E_DRAGCANCEL, URHO3D_HANDLER(AccountChestDialog, HandleItemDragCancel));
    SubscribeToEvent(icon, E_DRAGEND, URHO3D_HANDLER(AccountChestDialog, HandleItemDragEnd));

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
}

void AccountChestDialog::Clear()
{
    Text* moneyText = GetChildStaticCast<Text>("MoneyText", true);
    moneyText->SetText("0 Drachma");
    uint16_t pos = 1;
    while (auto* cont = GetItemContainer(pos))
    {
        cont->RemoveAllChildren();
        ++pos;
    }
    initializted_ = false;
}
