#pragma once

#include <Urho3DAll.h>

class HealthBarPlain : public ProgressBar
{
    URHO3D_OBJECT(HealthBarPlain, ProgressBar)
public:
    static void RegisterObject(Context* context);

    HealthBarPlain(Context* context);
    ~HealthBarPlain() override = default;
    void GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor) override;
    void UpdateKnob();
    void SetValues(unsigned max, unsigned value);
};
