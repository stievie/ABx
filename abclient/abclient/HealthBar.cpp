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
    SetImageRect(IntRect(16, 0, 32, 16));
    SetBorder(IntRect(4, 4, 4, 4));

    SetMinHeight(25);
    SetMaxHeight(25);
    SetShowPercentText(false);
    SetAlignment(HA_LEFT, VA_CENTER);
    nameText_ = CreateChild<Text>();
    nameText_->SetAlignment(HA_LEFT, VA_CENTER);

    SetStyle("HealthBar");

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(HealthBar, HandleUpdate));
}

HealthBar::~HealthBar()
{
    UnsubscribeFromAllEvents();
}

void HealthBar::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (SharedPtr<Actor> a = actor_.Lock())
    {
        nameText_->SetText(a->name_);
        SetRange(static_cast<float>(a->stats_.maxHealth));
        SetValue(static_cast<float>(a->stats_.health));
    }

    nameText_->SetVisible(showName_);
}
