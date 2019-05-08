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
    void SubscribeToEvents() override;
    void CreateUI() override;
private:
    bool loggingIn_;
    SharedPtr<LineEdit> nameEdit_;
    SharedPtr<LineEdit> passEdit_;
    SharedPtr<Button> button_;
    SharedPtr<Button> createAccountButton_;
    void CreateScene() override;
    void HandleLoginClicked(StringHash eventType, VariantMap& eventData);
    void HandleCreateAccountClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void DoLogin();
};
