#include "stdafx.h"
#include "HealthBar.h"

void HealthBar::RegisterObject(Context* context)
{
    context->RegisterFactory<HealthBar>();
}

HealthBar::HealthBar(Context* context) :
    ProgressBar(context),
    showName_(false)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());

    Texture2D* tex = cache->GetResource<Texture2D>("Textures/Fw-UI-Ex.png");
    SetTexture(tex);
    SetImageRect(IntRect(128, 16, 144, 32));
    SetBorder(IntRect(4, 4, 4, 4));

    SetMinHeight(25);
    SetMaxHeight(25);
    SetShowPercentText(false);
    SetAlignment(HA_LEFT, VA_CENTER);
    nameText_ = CreateChild<Text>();
    nameText_->SetAlignment(HA_LEFT, VA_CENTER);
    nameText_->SetStyleAuto();
    nameText_->SetPosition(5, 0);

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
        nameText_->SetText(actor->name_);
    else
        nameText_->SetText(String::EMPTY);
}

void HealthBar::HandleUpdate(StringHash, VariantMap&)
{
    if (SharedPtr<Actor> a = actor_.Lock())
    {
        SetRange(static_cast<float>(a->stats_.maxHealth));
        SetValue(static_cast<float>(a->stats_.health));
    }

    nameText_->SetVisible(showName_);
}
