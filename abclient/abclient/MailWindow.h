#pragma once

#include "MultiLineEdit.h"

class MailWindow : public UIElement
{
    URHO3D_OBJECT(MailWindow, UIElement);
public:
    static void RegisterObject(Context* context);

    MailWindow(Context* context);
    ~MailWindow()
    {
        UnsubscribeFromAllEvents();
    }

private:
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    SharedPtr<MultiLineEdit> previewEdit_;
};

