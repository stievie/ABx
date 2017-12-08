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
    uint32_t playerId_;
    Client::Client client_;
    Client::Client::ClientState lastState_;
    Client::CharList characters_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    static String GetProtocolErrorMessage(uint8_t err);

    FwClient(Context* context);
    ~FwClient();

    bool Start();
    void Stop();
    void Login(const String& name, const String& pass);
    void EnterWorld(const String& charName, const String& map);
    void Logout();

    void OnGetCharlist(const Client::CharList& chars) override;
    void OnEnterWorld(const std::string& mapName, uint32_t playerId) override;
    /// asio network error
    void OnNetworkError(const std::error_code& err) override;
    /// Protocol error, e.g. Login failed
    void OnProtocolError(uint8_t err) override;
    void OnSpawnObject(uint32_t id, float x, float y, float z, float rot, PropReadStream& data) override;
    void OnDespawnObject(uint32_t id) override;
    const Client::CharList& GetCharacters() const
    {
        return characters_;
    }
    int GetAvgPing() const
    {
        return client_.GetAvgPing();
    }
    int GetLastPing() const
    {
        return client_.GetLastPing();
    }

};

