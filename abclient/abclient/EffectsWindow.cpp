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


#include "EffectsWindow.h"
#include "FwClient.h"
#include "SkillManager.h"

void EffectsWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<EffectsWindow>();
}

EffectsWindow::EffectsWindow(Context* context) :
    UIElement(context),
    effectCount_(0)
{
    SetName("EffectsWindow");
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *file = cache->GetResource<XMLFile>("UI/EffectsWindow.xml");
    LoadChildXML(file->GetRoot(), nullptr);

    SetAlignment(HA_CENTER, VA_BOTTOM);
    SetPosition(0, -120);
    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_HORIZONTAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetMinSize(50, 50);
    SetVisible(false);
}

EffectsWindow::~EffectsWindow()
{
    UnsubscribeFromAllEvents();
}

void EffectsWindow::EffectAdded(uint32_t effectIndex, uint32_t ticks)
{
    SkillManager* sm = GetSubsystem<SkillManager>();
    const AB::Entities::Effect* effect = sm->GetEffectByIndex(effectIndex);
    if (!effect)
        return;

    String name = "Effect_" + String(effectIndex);
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    ++effectCount_;

    BorderImage* effectIcon = GetChildStaticCast<BorderImage>(name, true);
    if (!effectIcon)
        effectIcon = CreateChild<BorderImage>(name);
    effectIcon->SetVar("Index", effectIndex);
    effectIcon->SetVar("Ticks", ticks);
    effectIcon->SetSize(50, 50);
    effectIcon->SetMinSize(50, 50);
    effectIcon->SetOpacity(1.0f);

    Texture2D* icon = cache->GetResource<Texture2D>(String(effect->icon.c_str()));
    if (!icon)
        icon = cache->GetResource<Texture2D>("Textures/Skills/placeholder.png");
    icon->SetNumLevels(1);
    icon->SetMipsToSkip(QUALITY_LOW, 0);
    effectIcon->SetTexture(icon);
    effectIcon->SetFullImageRect();
    effectIcon->SetBorder(IntRect(4, 4, 4, 4));
    SetWidth(50 * effectCount_);
    SetVisible(effectCount_ != 0);
    UpdateLayout();
    UpdatePosition();
}

void EffectsWindow::EffectRemoved(uint32_t effectIndex)
{
    String name = "Effect_" + String(effectIndex);
    BorderImage* effectIcon = GetChildStaticCast<BorderImage>(name, true);
    if (effectIcon)
    {
        --effectCount_;
        RemoveChild(effectIcon);
        SetWidth(50 * effectCount_);
        SetVisible(effectCount_ != 0);
        UpdateLayout();
        UpdatePosition();
    }
}

void EffectsWindow::Clear()
{
    SetVisible(false);
    RemoveAllChildren();
    effectCount_ = 0;
    SetWidth(0);
}

void EffectsWindow::UpdatePosition()
{
    if (!IsVisible())
        return;
//    if (GetHorizontalAlignment() == HA_CENTER)
    {
        IntVector2 pos = GetPosition();
        pos.x_ = -200;
        SetPosition(pos);
    }
}
