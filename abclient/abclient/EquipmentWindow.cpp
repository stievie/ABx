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

    uint32_t primIndex = player.profession_->index;
    uint32_t secIndex = player.profession2_->index;
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
    auto* primAttribs = attribs->GetChild("PrimaryAttributes", true);
    auto* secAttribs = attribs->GetChild("SecondaryAttributes", true);
    primAttribs->RemoveAllChildren();
    secAttribs->RemoveAllChildren();
    auto* pDropdown = attribContainer->GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    unsigned p1Index = player.profession_->index;

    auto* sm = GetSubsystem<SkillManager>();
    auto* p1 = sm->GetProfessionByIndex(p1Index);
    if (!p1)
        // This shouldn't happen, all characters need a primary profession
        return;

    auto addAttribute = [&](UIElement* container, const AB::Entities::AttriInfo& attr)
    {
        auto a = sm->GetAttributeByIndex(attr.index);
        if(!a)
            return;

        auto* cont = container->CreateChild<UIElement>();
        cont->SetLayoutMode(LM_HORIZONTAL);
        auto* label = cont->CreateChild<Text>();
        label->SetText(String(a->name.c_str()));
        label->SetStyleAuto();
        label->SetFontSize(9);
        label->SetAlignment(HA_LEFT, VA_TOP);
        auto* edit = cont->CreateChild<LineEdit>();
        edit->SetMaxHeight(22);
        edit->SetTexture(tex);
        edit->SetImageRect(IntRect(48, 0, 64, 16));
        edit->SetBorder(IntRect(4, 4, 4, 4));
        edit->SetStyleAuto();
        edit->SetAlignment(HA_RIGHT, VA_TOP);
        edit->SetMaxWidth(50);
        edit->SetEditable(false);

        auto* spinner = cont->CreateChild<Spinner>("AttribSpinner");
        spinner->SetTexture(tex);
        spinner->SetImageRect(IntRect(48, 0, 64, 16));
        spinner->SetEdit(SharedPtr<LineEdit>(edit));
        spinner->SetFixedWidth(22);
        spinner->SetFixedHeight(22);

        spinner->SetMin(0);
        spinner->SetMax(20);
        spinner->SetStyleAuto();
        spinner->SetAlignment(HA_RIGHT, VA_TOP);
    };

    unsigned maxAttribCount = p1->attributeCount;
    for (const auto& attrib : p1->attributes)
        addAttribute(primAttribs, attrib);
    primAttribs->UpdateLayout();

    auto* selItem = pDropdown->GetSelectedItem();
    unsigned p2Index = selItem->GetVar("Int Value").GetUInt();
    if (p2Index > 0)
    {
        auto* p2 = sm->GetProfessionByIndex(p2Index);
        if (p2)
        {
            if (p2->attributeCount > maxAttribCount)
                maxAttribCount = p2->attributeCount;
            for (const auto& attrib : p2->attributes)
            {
                if (!attrib.primary)
                    addAttribute(secAttribs, attrib);
            }
        }
    }
    secAttribs->UpdateLayout();

    attribContainer->SetMaxHeight(maxAttribCount * 20);
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
