#include "stdafx.h"
#include "EquipmentWindow.h"
#include "Shortcuts.h"
#include "ShortcutEvents.h"

EquipmentWindow::EquipmentWindow(Context* context) :
    Window(context)
{
    SetName("SkillsWindow");

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/EquipmentWindow.xml");
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
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLEEQUIPWINDOW, "Equipment", true));

    SetSize(330, 420);
    SetPosition(10, 30);
    SetVisible(true);

    SetStyleAuto();

    SubscribeEvents();
}

EquipmentWindow::~EquipmentWindow()
{
    UnsubscribeFromAllEvents();
}

void EquipmentWindow::SubscribeEvents()
{

}

void EquipmentWindow::UpdateEquipment()
{

}
