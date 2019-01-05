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
