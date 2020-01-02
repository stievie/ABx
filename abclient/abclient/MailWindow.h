#pragma once

#include <AB/Entities/MailList.h>
#include "MultiLineEdit.h"
#include <Urho3DAll.h>

class MailWindow : public Window
{
    URHO3D_OBJECT(MailWindow, Window)
public:
    static void RegisterObject(Context* context);

    MailWindow(Context* context);
    ~MailWindow() override
    {
        UnsubscribeFromAllEvents();
    }
    void GetHeaders();
private:
    SharedPtr<ListView> mailList_;
    SharedPtr<MultiLineEdit> mailBody_;
    void SubscribeToEvents();
    void AddItem(const String& text, const String& style, const AB::Entities::MailHeader& header);
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleMailInboxMessage(StringHash eventType, VariantMap& eventData);
    void HandleMailReadMessage(StringHash eventType, VariantMap& eventData);
    void HandleNewClicked(StringHash eventType, VariantMap& eventData);
    void HandleReplyClicked(StringHash eventType, VariantMap& eventData);
    void HandleDeleteClicked(StringHash eventType, VariantMap& eventData);
    void HandleItemSelected(StringHash eventType, VariantMap& eventData);
    void HandleItemUnselected(StringHash eventType, VariantMap& eventData);
    void HandleNewMail(StringHash eventType, VariantMap& eventData);
};

