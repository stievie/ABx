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
#include <abshared/Mechanic.h>
#include "SkillBarWindow.h"
#include "WindowManager.h"
#include "FilePicker.h"

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
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));
    SetBringToFront(true);
    SetBringToBack(true);
    SetSize(450, 600);
    SetMinSize(450, 600);
    SetMaxSize(450, 600);
    SetPosition(10, 30);

    auto* attribContainer = GetChild("AttributesContanier", true);
    attribContainer->SetFixedHeight(100);

    DisableLayoutUpdate();
    ListView* lv = CreateChild<ListView>("SkillsList");
    lv->SetPosition(IntVector2(4, 180));
    lv->SetSize(IntVector2(GetWidth() - 8, GetHeight() - 184));
    lv->SetStyleAuto();
    lv->SetDragDropMode(DD_SOURCE);

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = GetChildStaticCast<Text>("CaptionText", true);
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLESKILLSWINDOW, "Skills", true));

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
    Button* loadButton = GetChildStaticCast<Button>("LoadButton", true);
    SubscribeToEvent(loadButton, E_RELEASED, URHO3D_HANDLER(SkillsWindow, HandleLoadClicked));
    Button* saveButton = GetChildStaticCast<Button>("SaveButton", true);
    SubscribeToEvent(saveButton, E_RELEASED, URHO3D_HANDLER(SkillsWindow, HandleSaveClicked));

    auto* professionDropdown = GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    SubscribeToEvent(professionDropdown, E_ITEMSELECTED, URHO3D_HANDLER(SkillsWindow, HandleProfessionSelected));

    SubscribeToEvent(Events::E_SET_ATTRIBUTEVALUE, URHO3D_HANDLER(SkillsWindow, HandleSetAttribValue));
    SubscribeToEvent(Events::E_SET_SECPROFESSION, URHO3D_HANDLER(SkillsWindow, HandleSetSecProfession));
    SubscribeToEvent(Events::E_ACTOR_SKILLS_CHANGED, URHO3D_HANDLER(SkillsWindow, HandleSkillsChanged));
}

void SkillsWindow::HandleFilePickerClosed(StringHash, VariantMap&)
{
    if (filePicker_)
        filePicker_.Reset();
}

void SkillsWindow::HandleSetAttribValue(StringHash, VariantMap& eventData)
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;

    using namespace Events::SetAttributeValue;
    if (player->gameId_ != eventData[P_OBJECTID].GetUInt())
        return;

    uint32_t attribIndex = eventData[P_ATTRIBINDEX].GetUInt();
    int value = eventData[P_VALUE].GetInt();
    unsigned remaining = eventData[P_REMAINING].GetUInt();
    SetAttributeValue(*player, attribIndex, value, remaining);
}

void SkillsWindow::HandleSkillsChanged(StringHash, VariantMap& eventData)
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;

    using namespace Events::ActorSkillsChanged;
    if (player->gameId_ != eventData[P_OBJECTID].GetUInt())
        return;
    UpdateAll();
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

void SkillsWindow::HandleLoadFileSelected(StringHash, VariantMap& eventData)
{
    if (!IsChangeable())
        return;

    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;

    using namespace FilePicked;
    const String& fileName = eventData[P_FILENAME].GetString();

    File file(context_, fileName);
    String templ = file.ReadLine().Trimmed();

    FwClient* client = GetSubsystem<FwClient>();
    client->LoadSkillTemplate(std::string(templ.CString()));
}

void SkillsWindow::HandleLoadClicked(StringHash, VariantMap&)
{
    if (!IsChangeable())
        return;

    if (filePicker_)
        filePicker_->Close();

    String path = AddTrailingSlash(Options::GetPrefPath()) + "skills/";

    if (!Options::CreateDir(path))
    {
        URHO3D_LOGERRORF("Failed to create directory %s", path.CString());
        return;
    }

    filePicker_ = new FilePicker(context_, path, FilePicker::Mode::Load);
    SubscribeToEvent(filePicker_, E_DIALOGCLOSE, URHO3D_HANDLER(SkillsWindow, HandleFilePickerClosed));
    SubscribeToEvent(filePicker_, E_FILEPICKED, URHO3D_HANDLER(SkillsWindow, HandleLoadFileSelected));
}

void SkillsWindow::HandleSaveFileSelected(StringHash, VariantMap& eventData)
{
    if (!IsChangeable())
        return;

    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;

    using namespace FilePicked;
    const String& fileName = eventData[P_FILENAME].GetString();
    auto* fileSystem = GetSubsystem<FileSystem>();
    if (fileSystem->FileExists(fileName))
        fileSystem->Delete(fileName);
    std::string templ = player->SaveSkillTemplate();
    File file(context_, fileName, FILE_WRITE);
    file.WriteLine(String(templ.c_str()));
}

void SkillsWindow::HandleSaveClicked(StringHash, VariantMap&)
{
    if (!IsChangeable())
        return;

    if (filePicker_)
        filePicker_->Close();

    String path = AddTrailingSlash(Options::GetPrefPath()) + "skills/";

    if (!Options::CreateDir(path))
    {
        URHO3D_LOGERRORF("Failed to create directory %s", path.CString());
        return;
    }

    filePicker_ = new FilePicker(context_, path, FilePicker::Mode::Save);
    SubscribeToEvent(filePicker_, E_DIALOGCLOSE, URHO3D_HANDLER(SkillsWindow, HandleFilePickerClosed));
    SubscribeToEvent(filePicker_, E_FILEPICKED, URHO3D_HANDLER(SkillsWindow, HandleSaveFileSelected));
}

void SkillsWindow::AddProfessions(const Actor& actor)
{
    auto* dropdown = GetChildStaticCast<DropDownList>("ProfessionDropdown", true);
    dropdown->RemoveAllItems();

    uint32_t primIndex = actor.profession_->index;
    String priName = String(actor.profession_->name.c_str());
    uint32_t secIndex = actor.profession2_->index;
    if (secIndex == 0)
    {
        dropdown->AddItem(CreateDropdownItem(priName +  "/(None)", 0));
    }
    unsigned selection = 0;
    const auto& profs = GetSubsystem<SkillManager>()->GetProfessions();
    for (const auto& prof : profs)
    {
        if (prof.second.index != 0 && prof.second.index != primIndex)
        {
            if (prof.second.index == secIndex)
                selection = dropdown->GetNumItems();
            dropdown->AddItem(CreateDropdownItem(priName + "/" + String(prof.second.name.c_str()), prof.second.index));
        }
    }
    dropdown->SetSelection(selection);
    dropdown->SetEnabled(IsChangeable());
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

    UpdateAttributes(*player);
    UpdateSkills(*player);
    UpdateLayout();
}

bool SkillsWindow::IsChangeable() const
{
    auto* lm = GetSubsystem<LevelManager>();
    if (!lm)
        return false;
    return AB::Entities::IsOutpost(lm->GetMapType());
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

void SkillsWindow::UpdateAttribsHeader(const Actor& actor)
{
    Text* header = GetChildStaticCast<Text>("AttributeHeaderText", true);
    std::stringstream ss;
    ss << "Attributes: " << actor.GetAvailableAttributePoints() << " available";
    header->SetText(String(ss.str().c_str()));
}

void SkillsWindow::SetAttributeValue(const Actor& actor, uint32_t index, int value, unsigned)
{
    auto* spinner = GetAttributeSpinner(index);
    if (!spinner)
        return;

    // Should also update the edit text
    spinner->SetValue(value);

    spinner->SetCanIncrease(actor.CanIncreaseAttributeRank(static_cast<Game::Attribute>(index)));
    UpdateAttribsHeader(actor);
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
        if (!a)
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
        spinner->SetMax(Game::MAX_PLAYER_ATTRIBUTE_RANK);
        spinner->SetValue(static_cast<int>(actor.GetAttributeRank(static_cast<Game::Attribute>(attr.index))));
        spinner->SetCanIncrease(actor.CanIncreaseAttributeRank(static_cast<Game::Attribute>(attr.index)));
        spinner->SetCanDecrease(IsChangeable());
        spinner->SetStyleAuto();
        spinner->SetAlignment(HA_RIGHT, VA_CENTER);
        SubscribeToEvent(spinner, E_VALUECHANGED, [this, attrIndex = attr.index](StringHash, VariantMap& eventData)
        {
            using namespace ValueChanged;
            int attrValue = eventData[P_VALUE].GetInt();

            auto* client = GetSubsystem<FwClient>();
            client->SetAttributeValue(attrIndex, static_cast<uint8_t>(attrValue));
        });
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

    UpdateAttribsHeader(actor);
}

void SkillsWindow::HandleSkillDragBegin(StringHash, VariantMap& eventData)
{
    using namespace DragBegin;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    auto* element = reinterpret_cast<UISelectable*>(eventData[P_ELEMENT].GetVoidPtr());
    auto* item = element->GetChildStaticCast<BorderImage>("SkillIcon", true);
    UIElement* root = GetSubsystem<UI>()->GetRoot();

    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    dragSkill_ = root->CreateChild<Window>();
    dragSkill_->SetLayout(LM_HORIZONTAL);
    dragSkill_->SetLayoutBorder(IntRect(4, 4, 4, 4));
    dragSkill_->SetTexture(tex);
    dragSkill_->SetImageRect(IntRect(48, 0, 64, 16));
    dragSkill_->SetBorder(IntRect(4, 4, 4, 4));
    dragSkill_->SetMinSize(40, 40);
    dragSkill_->SetMaxSize(40, 40);
    BorderImage* icon = dragSkill_->CreateChild<BorderImage>();
    icon->SetTexture(item->GetTexture());
    dragSkill_->SetPosition(item->GetPosition());
    dragSkill_->SetVar("SkillIndex", element->GetVar("SkillIndex"));

    int lx = eventData[P_X].GetInt();
    int ly = eventData[P_Y].GetInt();
    dragSkill_->SetPosition(IntVector2(lx, ly) - dragSkill_->GetSize() / 2);

    int buttons = eventData[P_BUTTONS].GetInt();
    element->SetVar("BUTTONS", buttons);
    dragSkill_->BringToFront();
}

void SkillsWindow::HandleSkillDragMove(StringHash, VariantMap& eventData)
{
    if (!dragSkill_)
        return;
    using namespace DragMove;
    dragSkill_->BringToFront();

    int buttons = eventData[P_BUTTONS].GetInt();
    auto* element = reinterpret_cast<UISelectable*>(eventData[P_ELEMENT].GetVoidPtr());
    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();
    int BUTTONS = element->GetVar("BUTTONS").GetInt();

    if (buttons == BUTTONS)
        dragSkill_->SetPosition(IntVector2(X, Y) - dragSkill_->GetSize() / 2);
}

void SkillsWindow::HandleSkillDragCancel(StringHash, VariantMap&)
{
    using namespace DragCancel;
    if (!dragSkill_)
        return;
    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragSkill_.Get());
    dragSkill_.Reset();
}

void SkillsWindow::HandleSkillDragEnd(StringHash, VariantMap& eventData)
{
    using namespace DragEnd;
    if (!dragSkill_)
        return;
    uint32_t skillIndex = dragSkill_->GetVar("SkillIndex").GetUInt();

    UIElement* root = GetSubsystem<UI>()->GetRoot();
    WindowManager* wm = GetSubsystem<WindowManager>();

    auto ssb = wm->GetWindow(WINDOW_SKILLBAR);
    if (auto* sb = dynamic_cast<SkillBarWindow*>(ssb.Get()))
    {
        int X = eventData[P_X].GetInt();
        int Y = eventData[P_Y].GetInt();
        sb->DropSkill(IntVector2(X, Y), skillIndex);
    }

    root->RemoveChild(dragSkill_.Get());
    dragSkill_.Reset();
}

void SkillsWindow::UpdateSkills(const Actor& actor)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* defTexture = cache->GetResource<Texture2D>("Textures/Skills/no_skill.png");
    auto* client = GetSubsystem<FwClient>();
    bool haveLocked = client->GetAccountType() >= AB::Entities::AccountTypeGamemaster;

    auto* sm = GetSubsystem<SkillManager>();
    ListView* lv = GetChildStaticCast<ListView>("SkillsList", true);

    std::string lastAttribUuid;

    auto createItem = [&](const AB::Entities::Skill& skill)
    {
        if (lastAttribUuid.compare(skill.attributeUuid) != 0)
        {
            // Add separator
            const AB::Entities::Attribute* attrib = sm->GetAttribute(skill.attributeUuid);
            String sepText;
            if (attrib)
                sepText = String(attrib->name.c_str());
            else
                sepText = "No Attribute";
            Text* sep = lv->CreateChild<Text>();
            sep->SetInternal(true);
            sep->SetText(sepText);
            lastAttribUuid = skill.attributeUuid;
            sep->SetStyle("SkillListAttributeHeaderText");
            lv->AddItem(sep);
        }

        UISelectable* item = lv->CreateChild<UISelectable>(String(skill.uuid.c_str()));
        item->SetVar("SkillIndex", skill.index);
        if (IsChangeable())
        {
            item->SetDragDropMode(DD_SOURCE);
            SubscribeToEvent(item, E_DRAGMOVE, URHO3D_HANDLER(SkillsWindow, HandleSkillDragMove));
            SubscribeToEvent(item, E_DRAGBEGIN, URHO3D_HANDLER(SkillsWindow, HandleSkillDragBegin));
            SubscribeToEvent(item, E_DRAGCANCEL, URHO3D_HANDLER(SkillsWindow, HandleSkillDragCancel));
            SubscribeToEvent(item, E_DRAGEND, URHO3D_HANDLER(SkillsWindow, HandleSkillDragEnd));
        }
        item->SetLayout(LM_HORIZONTAL);
        item->SetLayoutSpacing(4);
        BorderImage* skillIcon = item->CreateChild<BorderImage>("SkillIcon");
        skillIcon->SetInternal(true);
        skillIcon->SetMinSize(40, 40);
        skillIcon->SetMaxSize(40, 40);
        UIElement* textContainer = item->CreateChild<UIElement>();
        textContainer->SetLayout(LM_VERTICAL);
        textContainer->SetInternal(true);

        Text* name = textContainer->CreateChild<Text>();
        name->SetText(String(skill.name.c_str()));
        name->SetStyleAuto();
        name->SetInternal(true);
        Text* descr = textContainer->CreateChild<Text>();
        descr->SetText(String(skill.shortDescription.c_str()));
        descr->SetStyleAuto();
        descr->SetFontSize(8);
        descr->SetInternal(true);
        item->SetStyle("SkillListItem");

        Texture2D* icon = cache->GetResource<Texture2D>(String(skill.icon.c_str()));
        if (icon)
        {
            skillIcon->SetTexture(icon);
            skillIcon->SetImageRect(IntRect(0, 0, 256, 256));
            skillIcon->SetBorder(IntRect(4, 4, 4, 4));
            skillIcon->SetHoverOffset(IntVector2(4, 4));
        }
        else
        {
            skillIcon->SetTexture(defTexture);
            skillIcon->SetImageRect(IntRect(0, 0, 256, 256));
            skillIcon->SetBorder(IntRect(4, 4, 4, 4));
            skillIcon->SetHoverOffset(IntVector2(0, 0));
        }

        lv->AddItem(item);
    };

    auto haveAccess = [&](const AB::Entities::Skill& skill)
    {
        if (AB::Entities::HasSkillAccess(skill, AB::Entities::SkillAccessPlayer))
            return true;
        if (haveLocked)
        {
            // This player can have GM locked skills
            if (AB::Entities::HasSkillAccess(skill, AB::Entities::SkillAccessGM))
                return true;
        }
        return false;
    };

    lv->RemoveAllItems();
    // All Players have a primary profession
    sm->VisistSkillsByProfession(actor.profession_->uuid, [&](const AB::Entities::Skill& skill)
    {
        if (haveAccess(skill))
            createItem(skill);
        return Iteration::Continue;
    });
    // And may have a secondary profession
    if (actor.profession2_)
    {
        sm->VisistSkillsByProfession(actor.profession2_->uuid, [&](const AB::Entities::Skill& skill)
        {
            if (haveAccess(skill))
                createItem(skill);
            return Iteration::Continue;
        });
    }
    // Lastly add skills that all professions can have
    sm->VisistSkillsByProfession(AB::Entities::PROFESSION_NONE_UUID, [&](const AB::Entities::Skill& skill)
    {
        if (haveAccess(skill))
            createItem(skill);
        return Iteration::Continue;
    });
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
