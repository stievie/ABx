#pragma once

class MailWindow : public Window
{
    URHO3D_OBJECT(MailWindow, Window);
public:
    static void RegisterObject(Context* context);

    MailWindow(Context* context);
    ~MailWindow()
    {
        UnsubscribeFromAllEvents();
    }
private:
    void SubscribeToEvents();
};

