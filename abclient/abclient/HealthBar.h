#pragma once

#include "HealthBarPlain.h"
#include "Actor.h"

class HealthBar : public HealthBarPlain
{
    URHO3D_OBJECT(HealthBar, HealthBarPlain);
private:
    WeakPtr<Actor> actor_;
    SharedPtr<Text> nameText_;
    bool showName_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    HealthBar(Context* context);
    ~HealthBar();

    void SetActor(SharedPtr<Actor> actor);
    SharedPtr<Actor> GetActor();
    void SetShowName(bool value);
};

