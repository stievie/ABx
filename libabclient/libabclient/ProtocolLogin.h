#pragma once

#include "Protocol.h"

class ProtocolLogin : public Protocol
{
private:
    std::string accountName_;
    std::string password_;
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
public:
    ProtocolLogin(const std::string& account, const std::string& password);
    ~ProtocolLogin();
};

