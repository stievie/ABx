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
    typedef std::function<void()> EnterWorldCallback;
private:
    std::string accountName_;
    std::string accountPass_;
    std::string charName_;
    bool firstRevc_;
    EnterWorldCallback enterWorldCallback_;

    void SendLoginPacket();
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
    void OnError(const asio::error_code& err) override;

    bool ParseMessage(const std::shared_ptr<InputMessage>& message);
public:
    ProtocolGame();
    ~ProtocolGame();

    void Login(const std::string& accountName, const std::string& accountPass,
        std::string charName, const std::string& host, uint16_t port, const EnterWorldCallback& callback);
};

}
