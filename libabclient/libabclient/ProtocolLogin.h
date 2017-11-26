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

typedef std::vector<AccountCharacter> Charlist;

class ProtocolLogin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0x01 };
    enum { UseChecksum = true };
    typedef std::function<void()> CharlistCallback;
private:
    std::string accountName_;
    std::string password_;
    bool firstRecv_;
    CharlistCallback charlistCallback;
    void SendLoginPacket();
    void ParseMessage(const std::shared_ptr<InputMessage>& message);
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
public:
    ProtocolLogin();
    ~ProtocolLogin();

    void Login(std::string& host, uint16_t port,
        const std::string& account, const std::string& password, const CharlistCallback& callback);

    Charlist characters_;
};

}
