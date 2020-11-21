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


#include "DamageWindow.h"
#include "ServerEvents.h"
#include "LevelManager.h"
#include "Player.h"
#include "SkillManager.h"
#include <sa/time.h>

void DamageWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<DamageWindow>();
    context->RegisterFactory<DamageWindowItem>();
}

DamageWindowItem::DamageWindowItem(Context* context) :
    UIElement(context)
{
    SetLayoutMode(LM_HORIZONTAL);
    SetLayoutBorder({ 4, 4, 4, 4 });
}

DamageWindowItem::~DamageWindowItem()
{

}

bool DamageWindowItem::Initialize()
{
    RemoveAllChildren();
    auto* sm = GetSubsystem<SkillManager>();
    auto* skill = sm->GetSkillByIndex(index_);
    if (!skill)
        return false;

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Texture2D* icon = cache->GetResource<Texture2D>(String(skill->icon.c_str()));
    if (icon)
    {
        icon->SetNumLevels(1);
        icon->SetMipsToSkip(QUALITY_LOW, 0);
        BorderImage* skillIcon = CreateChild<BorderImage>("SkillIcon");
        skillIcon->SetInternal(true);
        skillIcon->SetTexture(icon);
        skillIcon->SetImageRect(IntRect(0, 0, 256, 256));
        skillIcon->SetBorder(IntRect(4, 4, 4, 4));
        skillIcon->SetMinSize(ICON_SIZE, ICON_SIZE);
        skillIcon->SetMaxSize(ICON_SIZE, ICON_SIZE);
    }

    text_ = CreateChild<Text>();
    text_->SetStyleAuto();
    text_->SetAlignment(HA_LEFT, VA_CENTER);
    text_->SetTextEffect(TE_STROKE);
    text_->SetEffectColor({ 0.5, 0.5, 0.5 });
    text_->SetColor(Color::BLACK);

    return true;
}

void DamageWindowItem::Touch()
{
    ++count_;
    auto* sm = GetSubsystem<SkillManager>();
    auto* skill = sm->GetSkillByIndex(index_);
    if (!skill)
        return;

    String txt;
    if (index_ != 0)
        txt = skill->name.c_str();
    else
        txt = "Melee";
    if (count_ > 1)
        txt.AppendWithFormat(" x %d", count_);
    text_->SetText(txt);
}

DamageWindow::DamageWindow(Context* context) :
    Window(context)
{
    SetName("DamageWindow");
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetLayoutMode(LM_VERTICAL);
    SetFocusMode(FM_NOTFOCUSABLE);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(0, 16, 16, 32));
    SetBorder(IntRect(4, 4, 4, 4));

    SetHeight(0);
    SetMinWidth(300);
    SetPosition({ 5, 100 });
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(DamageWindow, HandleUpdate));
    SubscribeToEvent(Events::E_OBJECTDAMAGED, URHO3D_HANDLER(DamageWindow, HandleObjectDamaged));
}

DamageWindow::~DamageWindow()
{
    UnsubscribeFromAllEvents();
}

void DamageWindow::Clear()
{
    RemoveAllChildren();
    items_.Clear();
}

void DamageWindow::HandleUpdate(StringHash, VariantMap&)
{
    if (items_.Size() == 0)
        return;

    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (player)
    {
        // Don't clear damage window when the player is dead. The Player may
        // want to know what killed him/her.
        if (player->IsDead())
            return;
    }

    bool changed = false;
    for (int i = static_cast<int>(items_.Size()) - 1; i >= 0; --i)
    {
        auto& item = items_.At(static_cast<unsigned>(i));
        if (item->damageTick_ + KEEP_ITEMS_MS < sa::time::tick())
        {
            RemoveChild(item.Get());
            items_.Erase(static_cast<unsigned>(i), 1);
            changed = true;
        }
    }
    if (changed)
    {
        SetHeight(static_cast<int>(items_.Size() * DamageWindowItem::ICON_SIZE));
        UpdateLayout();
    }
}

void DamageWindow::HandleObjectDamaged(StringHash, VariantMap& eventData)
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* player = lm->GetPlayer();
    if (!player)
        return;

    using namespace Events::ObjectDamaged;
    if (eventData[P_OBJECTID].GetUInt() != player->gameId_)
        return;

    uint32_t index = eventData[P_INDEX].GetUInt();
    bool changed = false;
    auto* item = FindItem(index);
    if (item == nullptr)
    {
        item = CreateChild<DamageWindowItem>();
        item->index_ = eventData[P_INDEX].GetUInt();
        if (item->Initialize())
        {
            items_.Push(SharedPtr<DamageWindowItem>(item));
            changed = true;
        }
        else
        {
            RemoveChild(item);
            item = nullptr;
        }
    }
    if (item)
    {
        item->value_ = eventData[P_DAMAGEVALUE].GetUInt();
        item->Touch();
        item->damageTick_ = sa::time::tick();
    }

    if (changed)
    {
        SetHeight(static_cast<int>(items_.Size() * DamageWindowItem::ICON_SIZE));
        UpdateLayout();
    }
}

DamageWindowItem* DamageWindow::FindItem(uint32_t index)
{
    for (auto& item : items_)
    {
        if (item->index_ == index)
            return item.Get();
    }
    return nullptr;
}
