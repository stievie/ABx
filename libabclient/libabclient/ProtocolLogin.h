#pragma once

#include "Protocol.h"
#include <string>
#include <stdint.h>
#include <vector>

namespace Client {

struct AccountCharacter
{
    uint32_t id;
    uint16_t level;
    std::string name;
};

class ProtocolLogin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0x01 };
    enum { UseChecksum = true };
private:
    std::string accountName_;
    std::string password_;
    bool firstRecv_;
    void SendLoginPacket();
    void ParseMessage(const std::shared_ptr<InputMessage>& message);
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
public:
    ProtocolLogin();
    ~ProtocolLogin();

    void Login(std::string& host, uint16_t port,
        const std::string& account, const std::string& password);

    std::vector<AccountCharacter> characters_;
};

}
