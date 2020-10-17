/**
 * Copyright 2020 Stefan Ascher
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

#include "SkillCostElement.h"

void SkillCostElement::RegisterObject(Context* context)
{
    context->RegisterFactory<SkillCostElement>();
}

SkillCostElement::SkillCostElement(Context* context) :
    UIElement(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    SetLayoutMode(LM_HORIZONTAL);
    SetFixedHeight(16);
}

SkillCostElement::~SkillCostElement()
{ }

void SkillCostElement::SetSkill(const AB::Entities::Skill& skill)
{
    RemoveAllChildren();
    CreateElements(skill);
}

void SkillCostElement::CreateElements(const AB::Entities::Skill& skill)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* texture = cache->GetResource<Texture2D>("Textures/Fw-UI-Ex.png");
    int width = 0;
    if (skill.activation != 0)
    {
        auto* container = CreateChild<UIElement>();
        container->SetLayoutMode(LM_HORIZONTAL);
        container->SetMaxWidth(50);
        auto* icon = container->CreateChild<BorderImage>();
        icon->SetSize({ 16, 16 });
        icon->SetMaxSize({ 16, 16 });
        icon->SetMinSize({ 16, 16 });
        icon->SetTexture(texture);
        icon->SetImageRect({ 128, 48, 144, 64 });
        auto* text = container->CreateChild<Text>();
        text->SetStyleAuto();
        text->SetFontSize(8);
        text->SetText(GetValueText(static_cast<float>(skill.activation) / 1000.0f));
        width += container->GetWidth();
    }
    if (skill.costAdrenaline != 0)
    {
        auto* container = CreateChild<UIElement>();
        container->SetLayoutMode(LM_HORIZONTAL);
        container->SetMaxWidth(50);
        auto* icon = container->CreateChild<BorderImage>();
        icon->SetSize({ 16, 16 });
        icon->SetMaxSize({ 16, 16 });
        icon->SetMinSize({ 16, 16 });
        icon->SetTexture(texture);
        icon->SetImageRect({ 192, 48, 192 + 16, 48 + 16 });
        auto* text = container->CreateChild<Text>();
        text->SetStyleAuto();
        text->SetFontSize(8);
        text->SetText(GetValueText(static_cast<float>(skill.costAdrenaline)));
        width += container->GetWidth();
    }
    if (skill.costEnergyRegen != 0)
    {
        auto* container = CreateChild<UIElement>();
        container->SetLayoutMode(LM_HORIZONTAL);
        container->SetMaxWidth(50);
        auto* icon = container->CreateChild<BorderImage>();
        icon->SetSize({ 16, 16 });
        icon->SetMaxSize({ 16, 16 });
        icon->SetMinSize({ 16, 16 });
        icon->SetTexture(texture);
        icon->SetImageRect({ 176, 48, 176 + 16, 48 + 16 });
        auto* text = container->CreateChild<Text>();
        text->SetStyleAuto();
        text->SetFontSize(8);
        text->SetText(GetValueText(static_cast<float>(skill.costEnergyRegen)));
        width += container->GetWidth();
    }
    if (skill.costOvercast != 0)
    {
        auto* container = CreateChild<UIElement>();
        container->SetLayoutMode(LM_HORIZONTAL);
        container->SetMaxWidth(50);
        auto* icon = container->CreateChild<BorderImage>();
        icon->SetSize({ 16, 16 });
        icon->SetMaxSize({ 16, 16 });
        icon->SetMinSize({ 16, 16 });
        icon->SetTexture(texture);
        icon->SetImageRect({ 208, 48, 208 + 16, 48 + 16 });
        auto* text = container->CreateChild<Text>();
        text->SetStyleAuto();
        text->SetFontSize(8);
        text->SetText(GetValueText(static_cast<float>(skill.costOvercast)));
        width += container->GetWidth();
        width += 30;
    }
    if (skill.costEnergy != 0)
    {
        auto* container = CreateChild<UIElement>();
        container->SetLayoutMode(LM_HORIZONTAL);
        container->SetMaxWidth(50);
        auto* icon = container->CreateChild<BorderImage>();
        icon->SetSize({ 16, 16 });
        icon->SetMaxSize({ 16, 16 });
        icon->SetMinSize({ 16, 16 });
        icon->SetTexture(texture);
        icon->SetImageRect({ 160, 48, 160 + 16, 48 + 16 });
        auto* text = container->CreateChild<Text>();
        text->SetStyleAuto();
        text->SetFontSize(8);
        text->SetText(GetValueText(static_cast<float>(skill.costEnergy)));
        width += container->GetWidth();
    }
    if (skill.costHp != 0)
    {
        auto* container = CreateChild<UIElement>();
        container->SetLayoutMode(LM_HORIZONTAL);
        container->SetMaxWidth(50);
        auto* icon = container->CreateChild<BorderImage>();
        icon->SetSize({ 16, 16 });
        icon->SetMaxSize({ 16, 16 });
        icon->SetMinSize({ 16, 16 });
        icon->SetTexture(texture);
        icon->SetImageRect({ 224, 48, 224 + 16, 48 + 16 });
        auto* text = container->CreateChild<Text>();
        text->SetStyleAuto();
        text->SetFontSize(8);
        text->SetText(GetValueText(static_cast<float>(skill.costHp)));
        width += container->GetWidth();
    }
    if (skill.recharge != 0)
    {
        auto* container = CreateChild<UIElement>();
        container->SetLayoutMode(LM_HORIZONTAL);
        container->SetMaxWidth(50);
        auto* icon = container->CreateChild<BorderImage>();
        icon->SetSize({ 16, 16 });
        icon->SetMaxSize({ 16, 16 });
        icon->SetMinSize({ 16, 16 });
        icon->SetTexture(texture);
        icon->SetImageRect({ 144, 48, 144 + 16, 48 + 16 });
        auto* text = container->CreateChild<Text>();
        text->SetStyleAuto();
        text->SetFontSize(8);
        text->SetText(GetValueText(static_cast<float>(skill.recharge) / 1000.0f));
        width += container->GetWidth();
    }
    if (width != 0)
    {
        SetFixedWidth(width + 8);
        SetMaxWidth(width + 8);
        SetMinWidth(width + 8);
    }
    UpdateLayout();
}

String SkillCostElement::GetValueText(float value)
{
    if (Equals(value, 0.250f))
        return "1/4";
    if (Equals(value, 0.500f))
        return "1/2";
    if (Equals(value, 0.750f))
        return "3/4";

    return String(static_cast<int>(value));
}
