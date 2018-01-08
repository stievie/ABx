#pragma once

#include "MultiLineEdit.h"

class MailWindow : public Object
{
    URHO3D_OBJECT(MailWindow, Object);
public:
    static void RegisterObject(Context* context);

    MailWindow(Context* context);
    ~MailWindow()
    {
        // This would remove the UI-element regardless of whether it is parented to UI's root or UI's modal-root
        if (window_)
            window_->Remove();
        UnsubscribeFromAllEvents();
    }
    /// Return dialog window.
    UIElement* GetWindow() const { return window_; }

private:
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    /// UI element containing the whole UI layout. Typically it is a Window element type.
    UIElement* window_;
    SharedPtr<MultiLineEdit> previewEdit_;
};

