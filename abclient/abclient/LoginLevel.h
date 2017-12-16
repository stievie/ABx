#pragma once

#include "BaseLevel.h"

class LoginLevel : public BaseLevel
{
    URHO3D_OBJECT(LoginLevel, BaseLevel);
public:
    /// Construct.
    LoginLevel(Context* context);
    void CreateCamera();
    void ShowError(const String& message, const String& title = "Error") override;

protected:
    virtual void SubscribeToEvents();
    virtual void CreateUI();

private:
    bool loggingIn_;
    LineEdit* nameEdit_;
    LineEdit* passEdit_;
    Button* button_;
    void CreateScene();
    void HandleLoginClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void DoLogin();
};
