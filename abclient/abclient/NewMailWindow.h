#pragma once

#include "MultiLineEdit.h"

class NewMailWindow : public Window
{
    URHO3D_OBJECT(NewMailWindow, Window);
public:
    static void RegisterObject(Context* context);

    NewMailWindow(Context* context);
    ~NewMailWindow()
    {
        UnsubscribeFromAllEvents();
    }
    void SetRecipient(const String& value);
    void SetSubject(const String& value);
private:
    SharedPtr<MultiLineEdit> mailBody_;
    SharedPtr<LineEdit> recipient_;
    SharedPtr<LineEdit> subject_;

    void SubscribeToEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleSendClicked(StringHash eventType, VariantMap& eventData);
};

