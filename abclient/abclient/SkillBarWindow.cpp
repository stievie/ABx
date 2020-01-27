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
#include "SkillBarWindow.h"
#include "FwClient.h"
#include "SkillManager.h"

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

void SkillBarWindow::SetSkills(const AB::SkillIndices& skills)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* defTexture = cache->GetResource<Texture2D>("Textures/UI.png");
    SkillManager* sm = GetSubsystem<SkillManager>();
    skills_ = skills;
    uint32_t i = 1;
    for (const auto& s : skills_)
    {
        bool iconSet = false;
        Button* btn = GetButtonFromIndex(i);
        const AB::Entities::Skill* skill = sm->GetSkillByIndex(s);
        if (skill)
        {
            Texture2D* icon = cache->GetResource<Texture2D>(String(skill->icon.c_str()));
            if (icon)
            {
                btn->SetTexture(icon);
                btn->SetImageRect(IntRect(0, 0, 256, 256));
                btn->SetBorder(IntRect(4, 4, 4, 4));
                btn->SetHoverOffset(IntVector2(4, 4));
                btn->SetPressedOffset(IntVector2(-4, -4));
                Text* tooltip = btn->GetChildStaticCast<Text>("SkillTooltipText", true);
                String tip = String(skill->name.c_str());
                tip += "\n" + String(skill->shortDescription.c_str());
                tooltip->SetText(tip);
                ToolTip* tt = btn->GetChildStaticCast<ToolTip>("SkillTooltip", true);
                tt->SetPosition(IntVector2(0, -(tooltip->GetHeight() + 10)));
                iconSet = true;
            }
            btn->SetEnabled(true);
        }
        else
            btn->SetEnabled(false);
        if (!iconSet)
        {
            btn->SetTexture(defTexture);
            btn->SetImageRect(IntRect(16, 0, 32, 16));
            btn->SetBorder(IntRect(4, 4, 4, 4));
            btn->SetHoverOffset(IntVector2(0, 0));
            Text* tooltip = btn->GetChildStaticCast<Text>("SkillTooltipText", true);
            tooltip->SetText(String::EMPTY);
        }
        ++i;
    }
}

void SkillBarWindow::SubscribeEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SkillBarWindow, HandleUpdate));
}

void SkillBarWindow::HandleUpdate(StringHash, VariantMap&)
{
}

void SkillBarWindow::HandleSkill1Clicked(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UseSkill(1);
}

void SkillBarWindow::HandleSkill2Clicked(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UseSkill(2);
}

void SkillBarWindow::HandleSkill3Clicked(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UseSkill(3);
}

void SkillBarWindow::HandleSkill4Clicked(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UseSkill(4);
}

void SkillBarWindow::HandleSkill5Clicked(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UseSkill(5);
}

void SkillBarWindow::HandleSkill6Clicked(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UseSkill(6);
}

void SkillBarWindow::HandleSkill7Clicked(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UseSkill(7);
}

void SkillBarWindow::HandleSkill8Clicked(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UseSkill(8);
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
