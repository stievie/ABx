#pragma once

#include <Urho3D/Urho3DAll.h>
#include "Client.h"

class FwClient : public Object
{
    URHO3D_OBJECT(FwClient, Object);
private:
    String accountName_;
    String accounbtPass_;
    bool loggedIn_;
    Client::Client client_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    FwClient(Context* context);
    ~FwClient();

    bool Start();
    void Stop();
    void Login(const String& name, const String& pass);
    void Logout();

    void onConnectionError(int message);
    void onConnectionClosed();
    void onEnterGame();
};

