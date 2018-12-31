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
    SetRange(0.0f);
    SetValue(0.0f);
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());

    Texture2D* tex = cache->GetResource<Texture2D>("Textures/Fw-UI-Ex.png");
    SetTexture(tex);

    SetMinHeight(23);
    SetMaxHeight(23);
    SetImageRect(IntRect(0, 16, 16, 32));
    SetBorder(IntRect(4, 4, 4, 4));
    SetShowPercentText(false);
    SetAlignment(HA_LEFT, VA_CENTER);
    nameText_ = CreateChild<Text>();
    nameText_->SetInternal(true);
    nameText_->SetAlignment(HA_LEFT, VA_CENTER);
    nameText_->SetStyleAuto();
    nameText_->SetPosition(5, 0);
    nameText_->SetFontSize(10);

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

void HealthBar::GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor)
{
    static const IntVector2 selOffset(0, 16);
    ProgressBar::GetBatches(batches, vertexData, currentScissor, selected_ ? selOffset : IntVector2::ZERO);
}

void HealthBar::HandleUpdate(StringHash, VariantMap&)
{
    if (SharedPtr<Actor> a = actor_.Lock())
    {
        SetRange(static_cast<float>(a->stats_.maxHealth));
        SetValue(static_cast<float>(a->stats_.health));
        if (a->stats_.health == 0)
        {
        }
    }

    nameText_->SetVisible(showName_);
}
