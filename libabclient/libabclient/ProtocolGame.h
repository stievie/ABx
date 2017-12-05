#pragma once

#include "Protocol.h"
#include <string>
#include <stdint.h>

namespace Client {

class ProtocolGame : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
    typedef std::function<void(const std::string& mapName)> EnterWorldCallback;
    typedef std::function<void(int)> PingCallback;
private:
    std::string accountName_;
    std::string accountPass_;
    std::string charName_;
    std::string map_;
    int64_t pingTick_;
    int lastPing_;
    bool firstRevc_;
    EnterWorldCallback enterWorldCallback_;
    PingCallback pingCallback_;

    void SendLoginPacket();
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
    void OnError(const asio::error_code& err) override;

    void ParseMessage(const std::shared_ptr<InputMessage>& message);
    void ParseError(const std::shared_ptr<InputMessage>& message);
    void ParseEnterWorld(const std::shared_ptr<InputMessage>& message);
    void ParsePong(const std::shared_ptr<InputMessage>& message);
    void ParseUpdate(const std::shared_ptr<InputMessage>& message);
public:
    ProtocolGame();
    ~ProtocolGame();

    void Login(const std::string& accountName, const std::string& accountPass,
        const std::string& charName, const std::string& map, const std::string& host, uint16_t port,
        const EnterWorldCallback& callback);
    void Logout();
    void Ping(const PingCallback& callback);
};

}
