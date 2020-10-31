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


#include "SkillBarWindow.h"
#include "Actor.h"
#include "FwClient.h"
#include "LevelManager.h"
#include "SkillCostElement.h"
#include "SkillManager.h"
#include "TemplateEvaluator.h"
#include "WindowManager.h"

void SkillBarWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<SkillBarWindow>();
}

SkillBarWindow::SkillBarWindow(Context* context) :
    Window(context),
    skills_()
{
    SetName("SkillBar");
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *file = cache->GetResource<XMLFile>("UI/SkillBarWindow.xml");
    LoadChildXML(file->GetRoot(), nullptr);

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(false);
    SetMovable(false);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));

    SetSize(400, 50);

    SetVisible(true);
    SetPosition(0, 0);
    SetAlignment(HA_CENTER, VA_BOTTOM);

    SetStyleAuto();

    skill1_ = GetChildStaticCast<Button>("Skill1", true);
    skill2_ = GetChildStaticCast<Button>("Skill2", true);
    skill3_ = GetChildStaticCast<Button>("Skill3", true);
    skill4_ = GetChildStaticCast<Button>("Skill4", true);
    skill5_ = GetChildStaticCast<Button>("Skill5", true);
    skill6_ = GetChildStaticCast<Button>("Skill6", true);
    skill7_ = GetChildStaticCast<Button>("Skill7", true);
    skill8_ = GetChildStaticCast<Button>("Skill8", true);

    ResetSkillButtons();

    SubscribeToEvent(skill1_, E_RELEASED, URHO3D_HANDLER(SkillBarWindow, HandleSkill1Clicked));
    SubscribeToEvent(skill2_, E_RELEASED, URHO3D_HANDLER(SkillBarWindow, HandleSkill2Clicked));
    SubscribeToEvent(skill3_, E_RELEASED, URHO3D_HANDLER(SkillBarWindow, HandleSkill3Clicked));
    SubscribeToEvent(skill4_, E_RELEASED, URHO3D_HANDLER(SkillBarWindow, HandleSkill4Clicked));
    SubscribeToEvent(skill5_, E_RELEASED, URHO3D_HANDLER(SkillBarWindow, HandleSkill5Clicked));
    SubscribeToEvent(skill6_, E_RELEASED, URHO3D_HANDLER(SkillBarWindow, HandleSkill6Clicked));
    SubscribeToEvent(skill7_, E_RELEASED, URHO3D_HANDLER(SkillBarWindow, HandleSkill7Clicked));
    SubscribeToEvent(skill8_, E_RELEASED, URHO3D_HANDLER(SkillBarWindow, HandleSkill8Clicked));
    SubscribeEvents();
}

SkillBarWindow::~SkillBarWindow()
{
    UnsubscribeFromAllEvents();
}

void SkillBarWindow::SetActor(SharedPtr<Actor> actor)
{
    actor_ = actor;
}

void SkillBarWindow::UpdateSkill(unsigned pos, uint32_t index)
{
    auto actor = actor_.Lock();
    if (!actor)
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* defTexture = cache->GetResource<Texture2D>("Textures/Skills/no_skill_disabled.png");
    defTexture->SetNumLevels(1);
    defTexture->SetMipsToSkip(QUALITY_LOW, 0);

    SkillManager* sm = GetSubsystem<SkillManager>();

    TemplateEvaluator templEval(*actor);

    bool iconSet = false;
    Button* btn = GetButtonFromIndex(pos);
    btn->SetVar("SkillPos", pos);
    const AB::Entities::Skill* skill = sm->GetSkillByIndex(index);
    if (skill && skill->index != 0)
    {
        btn->SetVar("SkillIndex", skill->index);
        Texture2D* icon = cache->GetResource<Texture2D>(String(skill->icon.c_str()));
        if (icon)
        {
            icon->SetNumLevels(1);
            icon->SetMipsToSkip(QUALITY_LOW, 0);
            btn->SetTexture(icon);
            btn->SetFullImageRect();
            btn->SetBorder(IntRect(4, 4, 4, 4));
            if (!IsChangeable())
            {
                btn->SetHoverOffset(IntVector2(4, 4));
                btn->SetPressedOffset(IntVector2(-4, -4));
            }
            else
            {
                btn->SetHoverOffset(IntVector2(0, 0));
                btn->SetPressedOffset(IntVector2(0, 0));
            }
            iconSet = true;
        }
        Text* skillName = btn->GetChildStaticCast<Text>("SkillName", true);
        skillName->SetText(String(skill->name.c_str()));
        UIElement* skillCostContainer = btn->GetChildStaticCast<UIElement>("SkillCost", true);
        skillCostContainer->RemoveAllChildren();
        SkillCostElement* skillCost = skillCostContainer->CreateChild<SkillCostElement>();
        skillCost->SetSkill(*skill);

        Text* skillDescription = btn->GetChildStaticCast<Text>("SkillDescription", true);
        skillDescription->SetText(String(templEval.Evaluate(skill->description).c_str()));
        Window* tooltipWindow = btn->GetChildStaticCast<Window>("TooltipWindow", true);
        ToolTip* tt = btn->GetChildStaticCast<ToolTip>("SkillTooltip", true);
        tt->SetPosition(IntVector2(0, -(tooltipWindow->GetHeight() + 10)));
        tt->SetEnabled(true);
        btn->SetEnabled(true);
        btn->UpdateLayout();
        tooltipWindow->SetVisible(true);
    }
    else
    {
        btn->SetVar("SkillIndex", 0);
        ToolTip* tt = btn->GetChildStaticCast<ToolTip>("SkillTooltip", true);
        tt->SetEnabled(false);
        btn->SetEnabled(false);
        Window* tooltipWindow = btn->GetChildStaticCast<Window>("TooltipWindow", true);
        tooltipWindow->SetVisible(false);
    }
    if (!iconSet)
    {
        btn->SetTexture(defTexture);
        btn->SetFullImageRect();
        btn->SetBorder(IntRect(4, 4, 4, 4));
        btn->SetHoverOffset(IntVector2(0, 0));
        btn->SetPressedOffset(IntVector2(0, 0));
    }
}

void SkillBarWindow::SetSkills(const Game::SkillIndices& skills)
{
    ResetSkillButtons();
    auto actor = actor_.Lock();
    if (!actor)
        return;

    skills_ = skills;
    uint32_t i = 1;
    for (const auto& s : skills_)
    {
        UpdateSkill(i, s);
        ++i;
    }
}

void SkillBarWindow::DropSkill(const IntVector2& pos, uint32_t skillIndex)
{
    IntRect screenRect(GetScreenPosition(), GetScreenPosition() + GetSize());
    if (!screenRect.IsInside(pos))
        return;

    IntVector2 clientPos = pos - GetScreenPosition();
    unsigned skillPos = GetSkillPosFromClientPos(clientPos);
    if (skillPos > Game::PLAYER_MAX_SKILLS - 1)
        return;
    auto* client = GetSubsystem<FwClient>();
    client->EquipSkill(skillIndex, static_cast<uint8_t>(skillPos));
}

void SkillBarWindow::SubscribeEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SkillBarWindow, HandleUpdate));
    SubscribeToEvent(Events::E_SET_SKILL, URHO3D_HANDLER(SkillBarWindow, HandleSetSkill));
    for (unsigned i = 1; i <= Game::PLAYER_MAX_SKILLS; ++i)
    {
        auto* button = GetButtonFromIndex(i);
        SubscribeToEvent(button, E_DRAGMOVE, URHO3D_HANDLER(SkillBarWindow, HandleSkillDragMove));
        SubscribeToEvent(button, E_DRAGBEGIN, URHO3D_HANDLER(SkillBarWindow, HandleSkillDragBegin));
        SubscribeToEvent(button, E_DRAGCANCEL, URHO3D_HANDLER(SkillBarWindow, HandleSkillDragCancel));
        SubscribeToEvent(button, E_DRAGEND, URHO3D_HANDLER(SkillBarWindow, HandleSkillDragEnd));
    }
}

void SkillBarWindow::HandleUpdate(StringHash, VariantMap&)
{
}

void SkillBarWindow::HandleSkill1Clicked(StringHash, VariantMap&)
{
    if (IsUseable())
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->UseSkill(1);
    }
    else if (IsChangeable())
        ShowSkillsWindow();
}

void SkillBarWindow::HandleSkill2Clicked(StringHash, VariantMap&)
{
    if (IsUseable())
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->UseSkill(2);
    }
    else if (IsChangeable())
        ShowSkillsWindow();
}

void SkillBarWindow::HandleSkill3Clicked(StringHash, VariantMap&)
{
    if (IsUseable())
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->UseSkill(3);
    }
    else if (IsChangeable())
        ShowSkillsWindow();
}

void SkillBarWindow::HandleSkill4Clicked(StringHash, VariantMap&)
{
    if (IsUseable())
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->UseSkill(4);
    }
    else if (IsChangeable())
        ShowSkillsWindow();
}

void SkillBarWindow::HandleSkill5Clicked(StringHash, VariantMap&)
{
    if (IsUseable())
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->UseSkill(5);
    }
    else if (IsChangeable())
        ShowSkillsWindow();
}

void SkillBarWindow::HandleSkill6Clicked(StringHash, VariantMap&)
{
    if (IsUseable())
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->UseSkill(6);
    }
    else if (IsChangeable())
        ShowSkillsWindow();
}

void SkillBarWindow::HandleSkill7Clicked(StringHash, VariantMap&)
{
    if (IsUseable())
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->UseSkill(7);
    }
    else if (IsChangeable())
        ShowSkillsWindow();
}

void SkillBarWindow::HandleSkill8Clicked(StringHash, VariantMap&)
{
    if (IsUseable())
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->UseSkill(8);
    }
    else if (IsChangeable())
        ShowSkillsWindow();
}

void SkillBarWindow::HandleSetSkill(StringHash, VariantMap& eventData)
{
    using namespace Events::SetSkill;
    uint32_t id = eventData[P_OBJECTID].GetUInt();
    if (id != actor_->gameId_)
        return;
    uint32_t skillIndex = eventData[P_SKILLINDEX].GetUInt();
    unsigned skillPos = eventData[P_SKILLPOS].GetUInt();
    UpdateSkill(skillPos + 1, skillIndex);
}

Button* SkillBarWindow::GetButtonFromIndex(uint32_t index)
{
    switch (index)
    {
    case 1:
        return skill1_;
    case 2:
        return skill2_;
    case 3:
        return skill3_;
    case 4:
        return skill4_;
    case 5:
        return skill5_;
    case 6:
        return skill6_;
    case 7:
        return skill7_;
    case 8:
        return skill8_;
    default:
        return nullptr;
    }
}

void SkillBarWindow::ResetSkillButtons()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* defTexture = cache->GetResource<Texture2D>("Textures/UI.png");
    for (uint32_t i = 1; i <= Game::PLAYER_MAX_SKILLS; ++i)
    {
        Button* btn = GetButtonFromIndex(i);
        ToolTip* tt = btn->GetChildStaticCast<ToolTip>("SkillTooltip", true);
        Window* tooltipWindow = btn->GetChildStaticCast<Window>("TooltipWindow", true);
        tooltipWindow->SetVisible(false);
        tt->SetEnabled(false);
        btn->SetTexture(defTexture);
        btn->SetImageRect(IntRect(16, 0, 32, 16));
        btn->SetBorder(IntRect(4, 4, 4, 4));
        btn->SetHoverOffset(IntVector2(0, 0));
        btn->SetEnabled(false);
    }
}

IntVector2 SkillBarWindow::GetButtonSize() const
{
    return skill1_->GetSize();
}

unsigned SkillBarWindow::GetSkillPosFromClientPos(const IntVector2& clientPos)
{
    return clientPos.x_ / GetButtonSize().x_;
}

bool SkillBarWindow::IsChangeable() const
{
    auto* lm = GetSubsystem<LevelManager>();
    if (!lm)
        return false;
    return AB::Entities::IsOutpost(lm->GetMapType());
}

bool SkillBarWindow::IsUseable() const
{
    auto* lm = GetSubsystem<LevelManager>();
    if (!lm)
        return false;
    return AB::Entities::IsBattle(lm->GetMapType());
}


void SkillBarWindow::ShowSkillsWindow()
{
    auto* wm = GetSubsystem<WindowManager>();
    auto s = wm->GetWindow(WINDOW_SKILLS);
    if (!s->IsVisible())
    {
        VariantMap& e = GetEventDataMap();
        SendEvent(Events::E_SC_TOGGLESKILLSWINDOW, e);
    }
    s->BringToFront();
}

void SkillBarWindow::HandleSkillDragBegin(StringHash, VariantMap& eventData)
{
    if (!IsChangeable())
        return;

    using namespace DragBegin;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    auto* element = reinterpret_cast<Button*>(eventData[P_ELEMENT].GetVoidPtr());
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
    icon->SetTexture(element->GetTexture());
    dragSkill_->SetPosition(element->GetPosition());
    dragSkill_->SetVar("SkillIndex", element->GetVar("SkillIndex"));
    dragSkill_->SetVar("SkillPos", element->GetVar("SkillPos"));

    int lx = eventData[P_X].GetInt();
    int ly = eventData[P_Y].GetInt();
    dragSkill_->SetPosition(IntVector2(lx, ly) - dragSkill_->GetSize() / 2);

    int buttons = eventData[P_BUTTONS].GetInt();
    element->SetVar("BUTTONS", buttons);
    dragSkill_->BringToFront();
}

void SkillBarWindow::HandleSkillDragMove(StringHash, VariantMap& eventData)
{
    if (!IsChangeable())
        return;

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

void SkillBarWindow::HandleSkillDragCancel(StringHash, VariantMap&)
{
    if (!IsChangeable())
        return;

    using namespace DragCancel;
    if (!dragSkill_)
        return;
    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragSkill_.Get());
    dragSkill_.Reset();
}

void SkillBarWindow::HandleSkillDragEnd(StringHash, VariantMap& eventData)
{
    if (!IsChangeable())
        return;

    using namespace DragEnd;
    if (!dragSkill_)
        return;
    uint32_t skillIndex = dragSkill_->GetVar("SkillIndex").GetUInt();

    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();
    IntVector2 pos = IntVector2(X, Y);
    IntRect screenRect(GetScreenPosition(), GetScreenPosition() + GetSize());
    if (!screenRect.IsInside(pos))
    {
        // Dropping anywhere -> remove skill
        uint32_t skillPos = dragSkill_->GetVar("SkillPos").GetUInt() - 1;
        auto* client = GetSubsystem<FwClient>();
        client->EquipSkill(0, static_cast<uint8_t>(skillPos));
    }
    else
        DropSkill(pos, skillIndex);

    UIElement* root = GetSubsystem<UI>()->GetRoot();
    root->RemoveChild(dragSkill_.Get());
    dragSkill_.Reset();
}
