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
    SharedPtr<MultiLineEdit> previewEdit_;
};

