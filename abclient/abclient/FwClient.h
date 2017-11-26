#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )
#include "Client.h"

class FwClient : public Object
{
    URHO3D_OBJECT(FwClient, Object);
private:
    String accountName_;
    String accounbtPass_;
    bool loggedIn_;
    Client::Client client_;
    Client::Client::ClientState lastState_;
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
    void onSelectCharacter();
};

