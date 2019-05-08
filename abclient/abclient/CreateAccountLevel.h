#pragma once

#include "BaseLevel.h"

class CreateAccountLevel : public BaseLevel
{
    URHO3D_OBJECT(CreateAccountLevel, BaseLevel);
public:
    /// Construct.
    CreateAccountLevel(Context* context);
    void CreateCamera();
    void ShowError(const String& message, const String& title = "Error") override;

protected:
    void SubscribeToEvents() override;
    void CreateUI() override;
private:
    SharedPtr<LineEdit> nameEdit_;
    SharedPtr<LineEdit> passEdit_;
    SharedPtr<LineEdit> repeatPassEdit_;
    SharedPtr<LineEdit> emailEdit_;
    SharedPtr<LineEdit> accKeyEdit_;
    SharedPtr<Text> accKeyPlaceholder_;
    SharedPtr<Button> button_;
    void DoCreateAccount();
    void DoCancel();
    void CreateScene() override;
    void HandleCreateClicked(StringHash eventType, VariantMap& eventData);
    void HandleCancelClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleAccKeyFocused(StringHash eventType, VariantMap& eventData);
    void HandleAccKeyDefocused(StringHash eventType, VariantMap& eventData);
};
