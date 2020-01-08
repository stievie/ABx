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

Text* EquipmentWindow::CreateDropdownItem(const String& text, unsigned value)
{
    Text* result = new Text(context_);
    result->SetText(text);
    result->SetVar("Int Value", value);
    result->SetStyle("DropDownItemEnumText");
    return result;
}

void EquipmentWindow::CreateUI()
{
    auto* attribContainer = GetChild("AttributesContanier", true);

    auto* pDropdown = attribContainer->GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    pDropdown->GetPopup()->SetWidth(pDropdown->GetWidth());
}

void EquipmentWindow::SubscribeEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(EquipmentWindow, HandleCloseClicked));

    auto* attribContainer = GetChild("AttributesContanier", true);
    auto* professionDropdown = attribContainer->GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    SubscribeToEvent(professionDropdown, E_ITEMSELECTED, URHO3D_HANDLER(EquipmentWindow, HandleProfessionSelected));
}

void EquipmentWindow::HandleProfessionSelected(StringHash, VariantMap&)
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;
    UpdateAttributes(*player);
}

void EquipmentWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void EquipmentWindow::AddProfessions(const Player& player)
{
    auto* attribContainer = GetChild("AttributesContanier", true);

    auto* dropdown = attribContainer->GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    dropdown->RemoveAllItems();

    uint32_t primIndex = player.profession_ ? player.profession_->index : 0;
    uint32_t secIndex = player.profession2_ ? player.profession2_->index : 0;
    if (secIndex == 0)
    {
        dropdown->AddItem(CreateDropdownItem("(None)", 0));
    }
    dropdown->GetPopup()->SetWidth(dropdown->GetWidth());
    unsigned selection = 0;
    const auto& profs = GetSubsystem<SkillManager>()->GetProfessions();
    for (const auto& prof : profs)
    {
        if (prof.second.index != 0 && prof.second.index != primIndex)
        {
            if (prof.second.index == secIndex)
                selection = dropdown->GetNumItems();
            dropdown->AddItem(CreateDropdownItem(String(prof.second.name.c_str()), prof.second.index));
        }
    }
    dropdown->SetSelection(selection);
}

void EquipmentWindow::UpdateAttributes(const Player& player)
{
    (void)player;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");

    auto* attribContainer = GetChild("AttributesContanier", true);
    auto* attribs = attribContainer->GetChild("Attributes", true);
    attribs->RemoveAllChildren();
    auto* pDropdown = attribContainer->GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    unsigned p1Index = player.profession_->index;
    unsigned p2Index = pDropdown->GetSelection();

    auto* sm = GetSubsystem<SkillManager>();
    auto* p1 = sm->GetProfessionByIndex(p1Index);
    if (!p1)
        // This shouldn't happen, all characters need a primary profession
        return;

    auto addAttribute = [&](const AB::Entities::AttriInfo& attr)
    {
        auto a = sm->GetAttributeByIndex(attr.index);
        if(!a)
            return;

        auto* cont = attribs->CreateChild<UIElement>();
        cont->SetLayoutMode(LM_HORIZONTAL);
        auto* label = cont->CreateChild<Text>();
        label->SetText(String(a->name.c_str()));
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
        spinner->SetEdit(SharedPtr<LineEdit>(edit));
        spinner->SetFixedWidth(22);
        spinner->SetFixedHeight(22);

        spinner->SetMin(0);
        spinner->SetMax(20);
        spinner->SetStyleAuto();
    };

    for (const auto& attrib : p1->attributes)
    {
        addAttribute(attrib);
    }

    auto* p2 = sm->GetProfessionByIndex(p2Index);
    if (!p2)
        return;
    for (const auto& attrib : p2->attributes)
    {
        addAttribute(attrib);
    }
}

void EquipmentWindow::UpdateSkills(const Player& player)
{
    (void)player;
}
void EquipmentWindow::UpdateEquipment(const Player& player)
{
    (void)player;
}

void EquipmentWindow::UpdateAll()
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;
    AddProfessions(*player);
    UpdateAttributes(*player);
    UpdateSkills(*player);
    UpdateEquipment(*player);
}
