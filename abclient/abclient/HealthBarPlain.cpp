#include "stdafx.h"
#include "HealthBarPlain.h"

void HealthBarPlain::RegisterObject(Context* context)
{
    context->RegisterFactory<HealthBarPlain>();

    URHO3D_COPY_BASE_ATTRIBUTES(ProgressBar);
}

HealthBarPlain::HealthBarPlain(Context* context) :
    ProgressBar(context)
{
    SetRange(0.0f);
    SetValue(0.0f);
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());

    Texture2D* tex = cache->GetResource<Texture2D>("Textures/Fw-UI-Ex.png");
    SetTexture(tex);
    SetImageRect(IntRect(0, 16, 16, 32));
    SetBorder(IntRect(4, 4, 4, 4));

    SetShowPercentText(false);
}

void HealthBarPlain::GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor)
{
    static const IntVector2 selOffset(0, 16);
    ProgressBar::GetBatches(batches, vertexData, currentScissor, selected_ ? selOffset : IntVector2::ZERO);
}

void HealthBarPlain::UpdateKnob()
{
    knob_->SetVisible(!Equals(value_, 0.0f));
}

void HealthBarPlain::SetValues(unsigned max, unsigned value)
{
    SetRange(static_cast<float>(max));
    SetValue(static_cast<float>(value));
    UpdateKnob();
}
