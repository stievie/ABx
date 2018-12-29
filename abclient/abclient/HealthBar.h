#pragma once

#include "Actor.h"

class HealthBar : public ProgressBar
{
    URHO3D_OBJECT(HealthBar, ProgressBar);
private:
    WeakPtr<Actor> actor_;
    SharedPtr<Text> nameText_;
    bool selected_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    HealthBar(Context* context);
    ~HealthBar();

    void SetActor(SharedPtr<Actor> actor);
    SharedPtr<Actor> GetActor()
    {
        return actor_.Lock();
    }
    bool IsSelected() const { return selected_; }
    void SetSelected(bool value);

    void GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor) override;
    bool showName_;
};

