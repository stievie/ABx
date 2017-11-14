#pragma once

#include <stdint.h>
#include <string>
#include "Definitions.h"
#include "ProtocolAdmin.h"
#include <memory>
#include <thread>
#include <condition_variable>

class Client
{
private:
    uint16_t port_;
    std::string host_;
    std::shared_ptr<ProtocolAdmin> protocol_;
    std::thread pollThread_;
    std::thread keepAliveThread_;
    std::string errorMessage_;
    bool running_;
    enum RespStatus
    {
        Pending,
        Success,
        Failure
    };
public:
    Client();
    ~Client();

    void SetServer(const std::string& host, uint16_t port)
    {
        host_ = host;
        port_ = port;
    }

    const std::string& GetHost() const
    {
        return host_;
    }
    uint16_t GetPort() const
    {
        return port_;
    }
    void Connect(const std::string& pass);
    bool Disconnect();
    bool SendCommand(char cmdByte, char* command);
    bool IsConnected() const { return protocol_ && protocol_->IsConnected(); }
    bool IsLoggedIn() const { return protocol_ && protocol_->IsLoggedIn(); }
    const std::string& GetErrorMessage() const { return errorMessage_; }
};

