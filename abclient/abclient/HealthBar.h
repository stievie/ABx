#pragma once

#include "Actor.h"

class HealthBar : public ProgressBar
{
    URHO3D_OBJECT(HealthBar, ProgressBar);
private:
    WeakPtr<Actor> actor_;
    SharedPtr<Text> nameText_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    HealthBar(Context* context);
    ~HealthBar();

    void SetActor(SharedPtr<Actor> actor);

    bool showName_;
};

