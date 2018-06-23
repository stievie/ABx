#pragma once

#include "Actor.h"

class HealthBar : public ProgressBar
{
    URHO3D_OBJECT(HealthBar, ProgressBar);
private:
    WeakPtr<Actor> actor_;
    SharedPtr<Text> nameText_;
public:
    static void RegisterObject(Context* context);

    HealthBar(Context* context);
    ~HealthBar();

    void Update(float timeStep) override;
    void SetActor(SharedPtr<Actor> actor)
    {
        actor_ = actor;
    }

    bool showName_;
};

