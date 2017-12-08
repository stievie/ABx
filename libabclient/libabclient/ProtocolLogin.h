#pragma once

#include "Protocol.h"
#include <string>
#include <stdint.h>
#include <vector>
#include "Account.h"
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>

namespace Client {

class ProtocolLogin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = AB::ProtocolLoginId };
    enum { UseChecksum = true };
    typedef std::function<void(const CharList& chars)> CharlistCallback;
private:
    std::string accountName_;
    std::string password_;
    bool firstRecv_;
    CharlistCallback charlistCallback;
    void SendLoginPacket();
    void SendKeyExchange();
    void ParseMessage(const std::shared_ptr<InputMessage>& message);
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
public:
    ProtocolLogin();
    ~ProtocolLogin() {}

    void Login(std::string& host, uint16_t port,
        const std::string& account, const std::string& password,
        const CharlistCallback& callback);

    std::string gameHost_;
    uint16_t gamePort_;
};

}
