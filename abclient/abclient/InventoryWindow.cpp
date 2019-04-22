#include "stdafx.h"
#include "InventoryWindow.h"
#include "Shortcuts.h"
#include "AbEvents.h"
#include "FwClient.h"
#include "ItemsCache.h"

void InventoryWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<InventoryWindow>();
}

InventoryWindow::InventoryWindow(Context* context) :
    Window(context)
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

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = dynamic_cast<Text*>(GetChild("CaptionText", true));
    caption->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLEINVENTORYWINDOW, "Inventory", true));

    Text* moneyText = dynamic_cast<Text*>(GetChild("MoneyText", true));
    moneyText->SetText("0 Drachma");
    SetSize(260, 480);
    SetPosition(10, 30);
    SetVisible(true);

    SetStyleAuto();

    SubscribeEvents();
}

InventoryWindow::~InventoryWindow()
{
    UnsubscribeFromAllEvents();
}

void InventoryWindow::GetInventory()
{
    FwClient* net = context_->GetSubsystem<FwClient>();
    net->UpdateInventory();
}

void InventoryWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(InventoryWindow, HandleCloseClicked));
    SubscribeToEvent(AbEvents::E_INVENTORY, URHO3D_HANDLER(InventoryWindow, HandleInventory));
}

void InventoryWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void InventoryWindow::HandleInventory(StringHash, VariantMap&)
{
    FwClient* net = context_->GetSubsystem<FwClient>();
    const auto& items = net->GetInventoryItems();
    ItemsCache* itemsCache = GetSubsystem<ItemsCache>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

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
        BorderImage* container = GetItemContainer(item.pos);
        if (!container)
            continue;

        container->RemoveAllChildren();
        // For ToolTips we need a button
        Button* icon = container->CreateChild<Button>("Icon");
        icon->SetPosition(4, 4);
        icon->SetSize(container->GetSize() - IntVector2(8, 8));
        icon->SetMinSize(icon->GetSize());
        Texture2D* texture = cache->GetResource<Texture2D>(i->iconFile_);
        icon->SetTexture(texture);
        icon->SetFullImageRect();
        icon->SetLayoutMode(LM_FREE);
        if (item.count > 1)
        {
            Text* count = icon->CreateChild<Text>("Count");
            count->SetAlignment(HA_LEFT, VA_BOTTOM);
            count->SetPosition(0, 0);
            count->SetSize(10, icon->GetWidth());
            count->SetMinSize(10, icon->GetWidth());
            count->SetText(String(item.count));
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
            String text = item.count > 1 ? String(item.count) + " " : "";
            text += i->name_;
            ttText1->SetText(text);
            ttText1->SetStyleAuto();

            String text2 = String(item.count * item.value) + " Drachma";
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
    SetStyleAuto();
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
