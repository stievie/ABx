#pragma once

#include "Client.h"
#include "Receiver.h"
#include "Account.h"
#include "Game.h"
#include <AB/ProtocolCodes.h>
#include "Structs.h"

struct EventItem
{
    StringHash eventId;
    VariantMap eventData;
};

class FwClient : public Object, public Client::Receiver
{
    URHO3D_OBJECT(FwClient, Object);
private:
    String currentLevel_;
    bool levelReady_;
    Vector<EventItem> queuedEvents_;
    uint32_t playerId_;
    Client::Client client_;
    Client::Client::ClientState lastState_;
    Client::CharList characters_;
    Client::GameList games_;
    String currentCharacter_;
    bool loggedIn_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleLevelReady(StringHash eventType, VariantMap& eventData);
    void QueueEvent(StringHash eventType, VariantMap& eventData);
public:
    static String GetProtocolErrorMessage(uint8_t err);

    FwClient(Context* context);
    ~FwClient();

    uint32_t GetIp() const
    {
        if (loggedIn_)
            return client_.GetIp();
        return 0;
    }
    bool Start();
    void Stop();
    void Login(const String& name, const String& pass);
    void CreateAccount(const String& name, const String& pass, const String& email, const String& accKey);
    void CreatePlayer(const String& name, const String& prof, PlayerSex sex, bool isPvp);
    void EnterWorld(const String& charName, const String& map);
    void ChangeWorld(const String& map);
    void Logout();
    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void Command(AB::GameProtocol::CommandTypes type, const String& data);

    void OnGetCharlist(const Client::CharList& chars) override;
    void OnGetGamelist(const Client::GameList& games) override;
    void OnEnterWorld(const std::string& mapName, uint32_t playerId) override;
    /// asio network error
    void OnNetworkError(const std::error_code& err) override;
    /// Protocol error, e.g. Login failed
    void OnProtocolError(uint8_t err) override;
    void OnSpawnObject(uint32_t id, const Vec3& pos, const Vec3& scale, float rot,
        PropReadStream& data, bool existing) override;
    void OnDespawnObject(uint32_t id) override;
    void OnObjectPos(uint32_t id, const Vec3& pos) override;
    void OnObjectRot(uint32_t id, float rot, bool manual) override;
    void OnObjectStateChange(uint32_t id, AB::GameProtocol::CreatureState state) override;
    void OnAccountCreated() override;
    void OnPlayerCreated(const std::string& name, const std::string& map) override;
    void OnObjectSelected(uint32_t sourceId, uint32_t targetId) override;
    void OnServerMessage(AB::GameProtocol::ServerMessageType type,
        const std::string& senderName, const std::string& message) override;
    void OnChatMessage(AB::GameProtocol::ChatMessageChannel channel,
        uint32_t senderId, const std::string& senderName, const std::string& message) override;

    void SetState(Client::Client::ClientState state)
    {
        if (state == Client::Client::StateDisconnected || state == Client::Client::StateCreateAccount)
            loggedIn_ = false;
        client_.state_ = state;
    }
    const String& GetCurrentCharacter() const
    {
        return currentCharacter_;
    }
    const Client::CharList& GetCharacters() const
    {
        return characters_;
    }
    const Client::GameList& GetGames() const
    {
        return games_;
    }
    int GetAvgPing() const
    {
        return client_.GetAvgPing();
    }
    int GetLastPing() const
    {
        return client_.GetLastPing();
    }
    uint32_t GetPlayerId() const
    {
        return playerId_;
    }

    String accountName_;
    String accountPass_;
};

