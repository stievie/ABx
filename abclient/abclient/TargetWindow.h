#pragma once

#include "Actor.h"
#include "HealthBarPlain.h"

static const StringHash E_TARGETWINDOW_UNSELECT = StringHash("Target Window unselect object");

class TargetWindow : public UIElement
{
    URHO3D_OBJECT(TargetWindow, UIElement)
private:
    WeakPtr<Actor> target_;
    SharedPtr<Text> targetText_;
    SharedPtr<HealthBarPlain> healthBar_;
    void HandleClearTargetClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    TargetWindow(Context* context);
    ~TargetWindow() override;

    void SetTarget(SharedPtr<Actor> target);
};

