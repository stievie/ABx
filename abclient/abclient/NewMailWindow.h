#pragma once

#include "MultiLineEdit.h"
#include <Urho3DAll.h>

class NewMailWindow : public Window
{
    URHO3D_OBJECT(NewMailWindow, Window)
public:
    static void RegisterObject(Context* context);

    NewMailWindow(Context* context);
    ~NewMailWindow() override
    {
        UnsubscribeFromAllEvents();
    }
    void SetRecipient(const String& value);
    void SetSubject(const String& value);
    const String& GetSubject() const;
private:
    SharedPtr<MultiLineEdit> mailBody_;
    SharedPtr<LineEdit> recipient_;
    SharedPtr<LineEdit> subject_;

    void SubscribeToEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleSendClicked(StringHash eventType, VariantMap& eventData);
};

