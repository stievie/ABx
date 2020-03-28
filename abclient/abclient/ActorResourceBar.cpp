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

#include "stdafx.h"
#include "ActorResourceBar.h"
#include "Actor.h"

void ActorResourceBar::RegisterObject(Context* context)
{
    context->RegisterFactory<ActorResourceBar>();
    context->RegisterFactory<ActorHealthBar>();
    context->RegisterFactory<ActorEnergyBar>();
}

ActorResourceBar::ActorResourceBar(Context* context) :
    Window(context)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    SetLayoutMode(LM_VERTICAL);
    SetPivot(0, 0);
    SetOpacity(1.0f);
    SetResizable(true);
    SetMovable(true);
    SetPosition({ 0, 0 });
    SetSize({ 200, 10 });
    SetMinSize({ 200, 10 });
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(0, 0, 0, 0));

    bar_ = CreateChild<ValueBar>();
    bar_->selectable_ = false;

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ActorResourceBar, HandleUpdate));
}

ActorResourceBar::~ActorResourceBar()
{
    UnsubscribeFromAllEvents();
}

void ActorResourceBar::SetActor(SharedPtr<Actor> actor)
{
    actor_ = actor;
}

void ActorResourceBar::SetResourceType(ResourceType type)
{
    if (type_ == type)
        return;

    type_ = type;
    switch (type_)
    {
    case ResourceType::Health:
        bar_->SetStyle("HealthBar");
        break;
    case ResourceType::Energy:
        bar_->SetStyle("EnergyBar");
        break;
    case ResourceType::None:
        break;
    }
}

void ActorResourceBar::HandleUpdate(StringHash, VariantMap&)
{
    if (auto actor = actor_.Lock())
    {
        switch (type_)
        {
        case ResourceType::Health:
            bar_->SetValues(actor->stats_.maxHealth, actor->stats_.health);
            break;
        case ResourceType::Energy:
            bar_->SetValues(actor->stats_.maxEnergy, actor->stats_.energy);
            break;
        case ResourceType::None:
            break;
        }
    }
}

ActorHealthBar::ActorHealthBar(Context* context) :
    ActorResourceBar(context)
{
    SetName("ActorHealthBar");
    SetResourceType(ResourceType::Health);
}

ActorHealthBar::~ActorHealthBar()
{
}

ActorEnergyBar::ActorEnergyBar(Context* context) :
    ActorResourceBar(context)
{
    SetName("ActorEnergyBar");
    SetResourceType(ResourceType::Energy);
}

ActorEnergyBar::~ActorEnergyBar()
{
}
