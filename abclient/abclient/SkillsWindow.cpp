#include "stdafx.h"
#include "SkillsWindow.h"
#include "Shortcuts.h"
#include "Spinner.h"
#include "SkillManager.h"
#include "Player.h"
#include "LevelManager.h"
#include "TabGroup.h"
#include "ShortcutEvents.h"
#include "SkillManager.h"
#include "FwClient.h"

SkillsWindow::SkillsWindow(Context* context) :
    Window(context)
{
    SetName("SkillsWindow");

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/SkillsWindow.xml");
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
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLESKILLSWINDOW, "Skills", true));

    SetSize(330, 420);
    SetPosition(10, 30);
    SetVisible(true);

    SetStyleAuto();

    SubscribeEvents();
}

SkillsWindow::~SkillsWindow()
{
    UnsubscribeFromAllEvents();
}

Text* SkillsWindow::CreateDropdownItem(const String& text, unsigned value)
{
    Text* result = new Text(context_);
    result->SetText(text);
    result->SetVar("Int Value", value);
    result->SetStyle("DropDownItemEnumText");
    return result;
}

void SkillsWindow::SubscribeEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(SkillsWindow, HandleCloseClicked));

    auto* professionDropdown = GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    SubscribeToEvent(professionDropdown, E_ITEMSELECTED, URHO3D_HANDLER(SkillsWindow, HandleProfessionSelected));

    SubscribeToEvent(Events::E_SET_ATTRIBUTEVALUE, URHO3D_HANDLER(SkillsWindow, HandleSetAttribValue));
    SubscribeToEvent(Events::E_SET_SECPROFESSION, URHO3D_HANDLER(SkillsWindow, HandleSetSecProfession));
}

void SkillsWindow::HandleSetAttribValue(StringHash, VariantMap& eventData)
{
    using namespace Events::SetAttributeValue;
    uint32_t attribIndex = eventData[P_ATTRIBINDEX].GetUInt();
    int value = eventData[P_VALUE].GetInt();
    SetAttributeValue(attribIndex, value);
}

void SkillsWindow::HandleSetSecProfession(StringHash, VariantMap& eventData)
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;

    using namespace Events::SetSecProfession;
    if (player->gameId_ != eventData[P_OBJECTID].GetUInt())
        return;
    uint32_t index = eventData[P_PROFINDEX].GetUInt();
    SetProfessionIndex(index);
}

void SkillsWindow::HandleProfessionSelected(StringHash, VariantMap&)
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;

    auto* dropdown = GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    auto* selItem = dropdown->GetSelectedItem();
    unsigned p2Index = selItem->GetVar("Int Value").GetUInt();
    if (p2Index == 0)
        return;

    if (player->profession2_->index == p2Index)
        return;

    auto* client = GetSubsystem<FwClient>();
    // If success this will call SkillsWindow::SetProfessionIndex()
    client->SetSecondaryProfession(p2Index);
}

void SkillsWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void SkillsWindow::AddProfessions(const Actor& actor)
{
    auto* dropdown = GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    dropdown->RemoveAllItems();

    uint32_t primIndex = actor.profession_->index;
    uint32_t secIndex = actor.profession2_->index;
    if (secIndex == 0)
    {
        dropdown->AddItem(CreateDropdownItem("(None)", 0));
    }
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

void SkillsWindow::SetProfessionIndex(uint32_t index)
{
    // Called by the server
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;

    auto* dropdown = GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    for (unsigned i = 0; i < dropdown->GetNumItems(); ++i)
    {
        auto* item = dropdown->GetItem(i);
        if (item->GetVar("Int Value").GetUInt() == index)
        {
            if (dropdown->GetSelection() != i)
                dropdown->SetSelection(i);
            break;
        }
    }

    auto* sm = GetSubsystem<SkillManager>();
    player->profession2_ = sm->GetProfessionByIndex(index);
    UpdateAttributes(*player);
    UpdateSkills(*player);
    UpdateLayout();
}

UIElement* SkillsWindow::GetAttributeContainer(uint32_t index)
{
    auto* attribContainer = GetChild("AttributesContanier", true);
    auto* primAttribs = attribContainer->GetChild("PrimaryAttributes", true);
    auto* attr = primAttribs->GetChild("Attribute" + String(index));
    if (attr)
        return attr;

    auto* secAttribs = attribContainer->GetChild("SecondaryAttributes", true);
    auto* attr2 = secAttribs->GetChild("Attribute" + String(index));
    if (attr2)
        return attr2;

    return nullptr;
}

LineEdit* SkillsWindow::GetAttributeEdit(uint32_t index)
{
    auto* cont = GetAttributeContainer(index);
    if (!cont)
        return nullptr;

    auto* edit = cont->GetChildStaticCast<LineEdit>("AttributeEdit", true);
    return edit;
}

Spinner* SkillsWindow::GetAttributeSpinner(uint32_t index)
{
    auto* cont = GetAttributeContainer(index);
    if (!cont)
        return nullptr;

    auto* spinner = cont->GetChildStaticCast<Spinner>("AttributeSpinner", true);
    return spinner;
}

void SkillsWindow::SetAttributeValue(uint32_t index, int value)
{
    auto* spinner = GetAttributeSpinner(index);
    if (!spinner)
        return;

    // Should also update the edit text
    spinner->SetValue(value);
}

void SkillsWindow::UpdateAttributes(const Actor& actor)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");

    auto* attribContainer = GetChild("AttributesContanier", true);
    auto* primAttribs = attribContainer->GetChild("PrimaryAttributes", true);
    auto* secAttribs = attribContainer->GetChild("SecondaryAttributes", true);
    primAttribs->RemoveAllChildren();
    secAttribs->RemoveAllChildren();
    unsigned p1Index = actor.profession_->index;

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
        cont->SetVar("AttributeIndex", attr.index);
        cont->SetName("Attribute" + String(attr.index));
        cont->SetLayoutMode(LM_HORIZONTAL);
        auto* label = cont->CreateChild<Text>();
        label->SetText(String(a->name.c_str()));
        label->SetStyleAuto();
        label->SetFontSize(9);
        label->SetAlignment(HA_LEFT, VA_CENTER);
        auto* edit = cont->CreateChild<LineEdit>();
        edit->SetName("AttributeEdit");
        edit->SetMaxHeight(22);
        edit->SetTexture(tex);
        edit->SetImageRect(IntRect(48, 0, 64, 16));
        edit->SetBorder(IntRect(4, 4, 4, 4));
        edit->SetStyleAuto();
        edit->SetAlignment(HA_RIGHT, VA_CENTER);
        edit->SetMaxWidth(50);
        edit->SetEditable(false);

        auto* spinner = cont->CreateChild<Spinner>("AttribSpinner");
        spinner->SetName("AttributeSpinner");
        spinner->SetTexture(tex);
        spinner->SetImageRect(IntRect(48, 0, 64, 16));
        spinner->SetEdit(SharedPtr<LineEdit>(edit));
        spinner->SetFixedWidth(22);
        spinner->SetFixedHeight(22);

        spinner->SetMin(0);
        spinner->SetMax(20);
        spinner->SetStyleAuto();
        spinner->SetAlignment(HA_RIGHT, VA_CENTER);
    };

    for (const auto& attrib : p1->attributes)
        addAttribute(primAttribs, attrib);

    auto* p2 = actor.profession2_;
    if (p2)
    {
        for (const auto& attrib : p2->attributes)
        {
            if (!attrib.primary)
                addAttribute(secAttribs, attrib);
        }
    }

    attribContainer->SetFixedHeight(static_cast<int>(p1->attributeCount * 22));
    attribContainer->UpdateLayout();
    UpdateLayout();
}

void SkillsWindow::UpdateSkills(const Actor& actor)
{
    auto* sm = GetSubsystem<SkillManager>();
    ListView* lv = GetChildStaticCast<ListView>("SkillsList", true);
    lv->RemoveAllItems();
    sm->VisistSkillsByProfession(actor.profession_->uuid, [&](const AB::Entities::Skill& skill)
    {
        Text* item = lv->CreateChild<Text>(String(skill.uuid.c_str()));
        item->SetText(String(skill.name.c_str()));
        item->SetStyle("ListViewItemText");
        lv->AddItem(item);
        return Iteration::Continue;
    });
    if (actor.profession2_)
    {
        sm->VisistSkillsByProfession(actor.profession2_->uuid, [&](const AB::Entities::Skill& skill)
        {
            Text* item = lv->CreateChild<Text>(String(skill.uuid.c_str()));
            item->SetText(String(skill.name.c_str()));
            item->SetStyle("ListViewItemText");
            lv->AddItem(item);
            return Iteration::Continue;
        });
    }
}

void SkillsWindow::UpdateAll()
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;
    AddProfessions(*player);
    UpdateAttributes(*player);
    UpdateSkills(*player);
}
