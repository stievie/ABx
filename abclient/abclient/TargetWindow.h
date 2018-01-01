#pragma once

#include "GameObject.h"

class TargetWindow : public UIElement
{
    URHO3D_OBJECT(TargetWindow, UIElement);
private:
    WeakPtr<GameObject> target_;
    SharedPtr<Text> targetText_;
    void HandleClearTargetClicked(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    TargetWindow(Context* context);
    ~TargetWindow();

    void SetTarget(SharedPtr<GameObject> target);
};

