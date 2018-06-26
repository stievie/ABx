#pragma once

class MailWindow : public Object
{
    URHO3D_OBJECT(MailWindow, Object);
public:
    static void RegisterObject(Context* context);

    MailWindow(Context* context);
    ~MailWindow()
    {
        UnsubscribeFromAllEvents();
    }
    bool visible_;
private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void SubscribeToEvents();
};

