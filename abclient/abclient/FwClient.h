#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )
#include "Client.h"
#include "Receiver.h"
#include "Account.h"

class FwClient : public Object, public Client::Receiver
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

    void OnGetCharlist() override;
    void OnEnterWorld() override;
    /// asio network error
    void OnNetworkError(const std::error_code& err) override;
    /// Protocol error, e.g. Login failed
    void OnProtocolError(uint8_t err) override;
    const Client::Charlist& GetCharacters() const
    {
        return client_.GetCharacters();
    }

};

