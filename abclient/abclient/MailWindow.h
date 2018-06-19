#pragma once

#include "NuklearUI.h"

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

    char buffer_[256];
    struct nk_vec2 windowPos_;
    struct nk_vec2 windowSize_;
    struct nk_rect windowRect_;
};

