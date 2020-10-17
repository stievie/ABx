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

#include "TargetWindow.h"
#include "Actor.h"
#include "FwClient.h"

void TargetWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<TargetWindow>();
}

TargetWindow::TargetWindow(Context* context) :
    UIElement(context)
{
    SetName("TargetWindow");
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/TargetWindow.xml");
    LoadChildXML(chatFile->GetRoot(), nullptr);

    targetText_ = GetChildStaticCast<Text>("TargetText", true);
    SetAlignment(HA_CENTER, VA_TOP);
    healthBar_ = GetChildStaticCast<ValueBar>("TargetHealthBar", true);
    healthBar_->SetStyle("HealthBar");

    tradeButton_ = GetChildStaticCast<Button>("TradeButton", true);
    tradeButton_->SetVisible(false);
    tradeButton_->SetFocusMode(FM_NOTFOCUSABLE);

    Button* clearTarget = GetChildStaticCast<Button>("ClearTargetButton", true);
    SubscribeToEvent(clearTarget, E_RELEASED, URHO3D_HANDLER(TargetWindow, HandleClearTargetClicked));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(TargetWindow, HandleUpdate));
    SubscribeToEvent(Events::E_SET_SECPROFESSION, URHO3D_HANDLER(TargetWindow, HandleActorSkillsChanged));

    SubscribeToEvent(tradeButton_, E_RELEASED, URHO3D_HANDLER(TargetWindow, HandleTradeClicked));
}

TargetWindow::~TargetWindow()
{
    UnsubscribeFromAllEvents();
}

void TargetWindow::HandleClearTargetClicked(StringHash, VariantMap&)
{
    VariantMap& e = GetEventDataMap();
    SendEvent(E_TARGETWINDOW_UNSELECT, e);
}

void TargetWindow::HandleTradeClicked(StringHash, VariantMap&)
{
    if (!tradeButton_->IsVisible())
        return;
    if (SharedPtr<Actor> a = target_.Lock())
    {
        if (a->objectType_ != ObjectType::Player)
            return;

        auto* client = GetSubsystem<FwClient>();
        client->TradeRequest(a->gameId_);
    }
}

void TargetWindow::HandleUpdate(StringHash, VariantMap&)
{
    if (SharedPtr<Actor> a = target_.Lock())
        healthBar_->SetValues(a->stats_.maxHealth, a->stats_.health);
    else
        SetTarget(SharedPtr<Actor>());
}

void TargetWindow::HandleActorSkillsChanged(StringHash, VariantMap& eventData)
{
    if (auto a = target_.Lock())
    {
        using namespace Events::ActorSkillsChanged;
        if (a->gameId_ != eventData[P_OBJECTID].GetUInt())
            return;
        targetText_->SetText(a->GetClassLevelName());
    }
}

void TargetWindow::SetTarget(SharedPtr<Actor> target)
{
    target_ = target;
    if (target.NotNull())
    {
        targetText_->SetText(target->GetClassLevelName());
        if (target->objectType_ == ObjectType::Player)
            tradeButton_->SetVisible(true);
        else
            tradeButton_->SetVisible(false);
        SetVisible(true);
    }
    else
        SetVisible(false);
}
