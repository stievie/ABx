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
#include "HealthBar.h"

void HealthBar::RegisterObject(Context* context)
{
    context->RegisterFactory<HealthBar>();
}

HealthBar::HealthBar(Context* context) :
    HealthBarPlain(context),
    showName_(false)
{
    SetMinHeight(23);
    SetMaxHeight(23);
    SetAlignment(HA_LEFT, VA_CENTER);

    nameText_ = CreateChild<Text>();
    nameText_->SetInternal(true);
    nameText_->SetAlignment(HA_LEFT, VA_CENTER);
    nameText_->SetStyleAuto();
    nameText_->SetPosition(5, 0);
    nameText_->SetFontSize(10);
    nameText_->SetVisible(false);

    SetStyle("HealthBar");

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(HealthBar, HandleUpdate));
    SubscribeToEvent(Events::E_SET_SECPROFESSION, URHO3D_HANDLER(HealthBar, HandleActorSkillsChanged));
}

HealthBar::~HealthBar()
{
    UnsubscribeFromAllEvents();
}

void HealthBar::SetActor(SharedPtr<Actor> actor)
{
    actor_ = actor;
    if (actor)
        nameText_->SetText(actor->GetClassLevelName());
    else
        nameText_->SetText(String::EMPTY);
}

SharedPtr<Actor> HealthBar::GetActor()
{
    return actor_.Lock();
}

void HealthBar::SetShowName(bool value)
{
    if (showName_ != value)
    {
        showName_ = value;
        nameText_->SetVisible(showName_);
    }
}

void HealthBar::HandleUpdate(StringHash, VariantMap&)
{
    if (SharedPtr<Actor> a = actor_.Lock())
    {
        SetValues(a->stats_.maxHealth, a->stats_.health);
    }
}

void HealthBar::HandleActorSkillsChanged(StringHash, VariantMap& eventData)
{
    if (auto a = actor_.Lock())
    {
        using namespace Events::ActorSkillsChanged;
        if (a->gameId_ != eventData[P_OBJECTID].GetUInt())
            return;
        nameText_->SetText(a->GetClassLevelName());
    }
}
