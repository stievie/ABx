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
    virtual void SubscribeToEvents();
    virtual void CreateUI();

private:
    LineEdit* nameEdit_;
    LineEdit* passEdit_;
    LineEdit* repeatPassEdit_;
    LineEdit* emailEdit_;
    LineEdit* accKeyEdit_;
    Button* button_;
    void DoCreateAccount();
    void DoCancel();
    void CreateScene();
    void HandleCreateClicked(StringHash eventType, VariantMap& eventData);
    void HandleCancelClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
};
