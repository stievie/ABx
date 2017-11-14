#pragma once

#include "Protocol.h"

namespace Client {

class ProtocolGame : public Protocol
{
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;

    bool ParsePacket(uint8_t cmd, const std::shared_ptr<InputMessage>& message);
public:
    ProtocolGame();
    ~ProtocolGame();
};

}
