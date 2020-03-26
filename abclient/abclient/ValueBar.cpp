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
#include "ValueBar.h"

void ValueBar::RegisterObject(Context* context)
{
    context->RegisterFactory<ValueBar>();

    URHO3D_COPY_BASE_ATTRIBUTES(ProgressBar);
}

ValueBar::ValueBar(Context* context) :
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

void ValueBar::GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor)
{
    static const IntVector2 selOffset(0, 16);
    if (selectable_)
        return ProgressBar::GetBatches(batches, vertexData, currentScissor, selected_ ? selOffset : IntVector2::ZERO);

    return ProgressBar::GetBatches(batches, vertexData, currentScissor, IntVector2::ZERO);
}

void ValueBar::UpdateKnob()
{
    knob_->SetVisible(!Equals(value_, 0.0f));
}

void ValueBar::SetValues(unsigned max, unsigned value)
{
    SetRange(static_cast<float>(max));
    SetValue(static_cast<float>(value));
    UpdateKnob();
}
