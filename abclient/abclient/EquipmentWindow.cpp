#include "stdafx.h"
#include "EquipmentWindow.h"
#include "Shortcuts.h"
#include "Spinner.h"
#include "SkillManager.h"
#include "Player.h"
#include "LevelManager.h"

EquipmentWindow::EquipmentWindow(Context* context) :
    Window(context)
{
    SetName("EquipmentWindow");

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/SkillsEquipWindow.xml");
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

    CreateUI();

    SetStyleAuto();

    SubscribeEvents();
}

EquipmentWindow::~EquipmentWindow()
{
    UnsubscribeFromAllEvents();
}

Text* EquipmentWindow::CreateDropdownItem(const String& text, const String& value)
{
    Text* result = new Text(context_);
    result->SetText(text);
    result->SetVar("String Value", value);
    result->SetStyle("DropDownItemEnumText");
    return result;
}

void EquipmentWindow::CreateUI()
{
    auto* attribContainer = GetChild("AttributesContanier", true);

    auto* professionDropdown = attribContainer->GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    SubscribeToEvent(professionDropdown, E_ITEMSELECTED, URHO3D_HANDLER(EquipmentWindow, HandleProfessionSelected));

    professionDropdown->GetPopup()->SetWidth(professionDropdown->GetWidth());
    const auto& profs = GetSubsystem<SkillManager>()->GetProfessions();
    for (const auto& prof : profs)
    {
        if (prof.second.index != 0)
            professionDropdown->AddItem(CreateDropdownItem(String(prof.second.name.c_str()), String(prof.first.c_str())));
    }

    auto attribs = attribContainer->GetChild("Attributes", true);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    {
        auto* cont = attribs->CreateChild<UIElement>();
        cont->SetLayoutMode(LM_HORIZONTAL);
        auto* label = cont->CreateChild<Text>();
        label->SetText("Attribute");
        label->SetStyleAuto();
        auto* edit = cont->CreateChild<LineEdit>();
        edit->SetMaxHeight(22);
        edit->SetTexture(tex);
        edit->SetImageRect(IntRect(48, 0, 64, 16));
        edit->SetBorder(IntRect(4, 4, 4, 4));
        edit->SetStyleAuto();
        edit->SetCursorMovable(false);
        edit->SetTextCopyable(false);
        edit->SetTextSelectable(false);

        auto* spinner = cont->CreateChild<Spinner>("AttribSpinner");
        spinner->SetTexture(tex);
        spinner->SetImageRect(IntRect(48, 0, 64, 16));
        spinner->SetEdit(SharedPtr(edit));
        spinner->SetFixedWidth(22);
        spinner->SetFixedHeight(22);

        spinner->SetMin(0);
        spinner->SetMax(20);
        spinner->SetStyleAuto();
    }
}

void EquipmentWindow::SubscribeEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(EquipmentWindow, HandleCloseClicked));
}

void EquipmentWindow::HandleProfessionSelected(StringHash, VariantMap& eventData)
{
    (void)eventData;
    using namespace ItemSelected;
}

void EquipmentWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void EquipmentWindow::UpdateEquipment()
{

}
