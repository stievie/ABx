#pragma once

class HealthBarPlain : public ProgressBar
{
    URHO3D_OBJECT(HealthBarPlain, ProgressBar);
public:
    static void RegisterObject(Context* context);

    HealthBarPlain(Context* context);
    ~HealthBarPlain() = default;
    void GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor) override;
    void UpdateKnob();
    void SetValues(unsigned max, unsigned value);
};
