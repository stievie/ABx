#pragma once

#include "Protocol.h"
#include <string>
#include <stdint.h>

namespace Client {

class ProtocolGame : public Protocol
{
private:
    std::string accountName_;
    std::string accountPass_;
    bool firstRevc_;
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
    void OnError(const asio::error_code& err) override;

    bool ParseMessage(const std::shared_ptr<InputMessage>& message);
public:
    ProtocolGame();
    ~ProtocolGame();

    void Login(const std::string& accountName, const std::string& accountPass,
        const std::string& host, uint16_t port);
};

}
