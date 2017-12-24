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
    typedef std::function<void()> CreateAccountCallback;
private:
    enum ProtocolAction
    {
        ActionUnknown,
        ActionLogin,
        ActionCreateAccount
    };
    ProtocolAction action_;
    std::string accountName_;
    std::string password_;
    std::string email_;
    std::string accKey_;
    bool firstRecv_;
    CharlistCallback charlistCallback;
    CreateAccountCallback createAccCallback_;
    void SendLoginPacket();
    void SendCreateAccountPacket();
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
    void CreateAccount(std::string& host, uint16_t port,
        const std::string& account, const std::string& password,
        const std::string& email, const std::string& accKey,
        const CreateAccountCallback& callback);

    std::string gameHost_;
    uint16_t gamePort_;
};

}
