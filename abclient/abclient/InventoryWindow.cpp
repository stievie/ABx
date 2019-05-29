#include "stdafx.h"
#include "InventoryWindow.h"
#include "Shortcuts.h"
#include "AbEvents.h"
#include "FwClient.h"
#include "ItemsCache.h"
#include "Item.h"

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
    Text* caption = dynamic_cast<Text*>(GetChild("CaptionText", true));
    caption->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLEINVENTORYWINDOW, "Inventory", true));

    Text* moneyText = dynamic_cast<Text*>(GetChild("MoneyText", true));
    moneyText->SetText("0 Drachma");
    SetSize(260, 480);
    SetPosition(10, 30);
    SetVisible(true);

    SetStyleAuto();

    itemPopup_ = new Menu(context_);
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
        // Store
        Menu* item = popup->CreateChild<Menu>();
        item->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
        item->SetStyleAuto();
        Text* menuText = item->CreateChild<Text>();
        menuText->SetText("Store");
        menuText->SetStyle("EditorMenuText");
        item->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
        item->SetMinSize(menuText->GetSize() + IntVector2(4, 4));
        item->SetSize(item->GetMinSize());
        if (item->GetWidth() > width)
            width = item->GetWidth();
        if (item->GetHeight() > height)
            height = item->GetHeight();
        SubscribeToEvent(item, E_MENUSELECTED, URHO3D_HANDLER(InventoryWindow, HandleItemStoreSelected));
    }
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
    Text* moneyText = dynamic_cast<Text*>(GetChild("MoneyText", true));
    moneyText->SetText("0 Drachma");
    uint16_t pos = 1;
    while (auto cont = GetItemContainer(pos))
    {
        cont->RemoveAllChildren();
        ++pos;
    }
    initializted_ = false;
}

void InventoryWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(InventoryWindow, HandleCloseClicked));
    SubscribeToEvent(AbEvents::E_INVENTORY, URHO3D_HANDLER(InventoryWindow, HandleInventory));
    SubscribeToEvent(AbEvents::E_INVENTORYITEMUPDATE, URHO3D_HANDLER(InventoryWindow, HandleInventoryItemUpdate));
    SubscribeToEvent(AbEvents::E_INVENTORYITEMDELETE, URHO3D_HANDLER(InventoryWindow, HandleInventoryItemRemove));
}

void InventoryWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void InventoryWindow::SetItem(Item* item, const Client::InventoryItem& iItem)
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
    SubscribeToEvent(icon, E_CLICKEND, URHO3D_HANDLER(InventoryWindow, HandleItemClicked));
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

void InventoryWindow::HandleInventory(StringHash, VariantMap&)
{
    Clear();
    FwClient* net = context_->GetSubsystem<FwClient>();
    const auto& items = net->GetInventoryItems();
    ItemsCache* itemsCache = GetSubsystem<ItemsCache>();

    Text* moneyText = dynamic_cast<Text*>(GetChild("MoneyText", true));

    for (const auto& item : items)
    {
        if (item.type == AB::Entities::ItemTypeMoney)
        {
            moneyText->SetText(String(item.count) + " Drachma");
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
    using namespace AbEvents::InventoryItemUpdate;

    FwClient* net = context_->GetSubsystem<FwClient>();
    ItemsCache* itemsCache = GetSubsystem<ItemsCache>();

    uint16_t pos = static_cast<uint16_t>(eventData[P_ITEMPOS].GetUInt());

    const Client::InventoryItem& iItem = net->GetInventoryItem(pos);
    if (iItem.type == AB::Entities::ItemTypeUnknown)
        return;

    if (iItem.type == AB::Entities::ItemTypeMoney)
    {
        Text* moneyText = dynamic_cast<Text*>(GetChild("MoneyText", true));
        moneyText->SetText(String(iItem.count) + " Drachma");
        return;
    }
    Item* i = itemsCache->Get(iItem.index);
    if (!i)
        return;
    SetItem(i, iItem);
}

void InventoryWindow::HandleInventoryItemRemove(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::InventoryItemDelete;
    uint16_t pos = static_cast<uint16_t>(eventData[P_ITEMPOS].GetUInt());
    Client::InventoryItem item;
    item.type = AB::Entities::ItemTypeUnknown;
    item.pos = pos;
    SetItem(nullptr, item);
}

void InventoryWindow::HandleItemClicked(StringHash, VariantMap& eventData)
{
    using namespace ClickEnd;
    MouseButton button = static_cast<MouseButton>(eventData[P_BUTTON].GetUInt());
    Button* elem = dynamic_cast<Button*>(eventData[P_ELEMENT].GetPtr());
    if (!elem)
        return;
    if (button == MOUSEB_RIGHT)
    {
        int x = eventData[P_X].GetInt();
        int y = eventData[P_Y].GetInt();
        itemPopup_->SetPosition(x, y);
        itemPopup_->ShowPopup(true);
        itemPopup_->SetVar("ItemPos", elem->GetVar("POS").GetUInt());
    }
}

void InventoryWindow::HandleItemStoreSelected(StringHash, VariantMap& eventData)
{
    itemPopup_->ShowPopup(false);
    using namespace MenuSelected;
    Menu* sender = dynamic_cast<Menu*>(eventData[P_ELEMENT].GetPtr());
    if (!sender)
        return;
    unsigned pos = itemPopup_->GetVar("ItemPos").GetUInt();
    if (pos > std::numeric_limits<uint16_t>::max())
        return;
    FwClient* cli = GetSubsystem<FwClient>();
    cli->InventoryDestroyItem(static_cast<uint16_t>(pos));
}

void InventoryWindow::HandleItemDestroySelected(StringHash, VariantMap& eventData)
{
    itemPopup_->ShowPopup(false);
    using namespace MenuSelected;
    Menu* sender = dynamic_cast<Menu*>(eventData[P_ELEMENT].GetPtr());
    if (!sender)
        return;
    unsigned pos = itemPopup_->GetVar("ItemPos").GetUInt();
    if (pos > std::numeric_limits<uint16_t>::max())
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
    if (pos > std::numeric_limits<uint16_t>::max())
        return;
    FwClient* cli = GetSubsystem<FwClient>();
    cli->InventoryDropItem(static_cast<uint16_t>(pos));
}

BorderImage* InventoryWindow::GetItemContainer(uint16_t pos)
{
    // pos is 1-based
    unsigned rowIndex = (pos - 1) / 5;
    UIElement* container = dynamic_cast<UIElement*>(GetChild("Container", true));
    String name("ItemRow" + String(rowIndex + 1));
    UIElement* row = container->GetChild(name, true);
    if (!row)
    {
        return nullptr;
    }
    unsigned index = pos - (rowIndex * 5) - 1;
    BorderImage* result = dynamic_cast<BorderImage*>(row->GetChild(index));
    return result;
}
