#pragma once

#include "Protocol.h"
#include <string>
#include <stdint.h>

enum
{
    //
    AP_MSG_LOGIN = 1,
    AP_MSG_ENCRYPTION = 2,
    AP_MSG_KEY_EXCHANGE = 3,
    AP_MSG_COMMAND = 4,
    AP_MSG_PING = 5,
    AP_MSG_KEEP_ALIVE = 6,
    //
    AP_MSG_HELLO = 1,
    AP_MSG_KEY_EXCHANGE_OK = 2,
    AP_MSG_KEY_EXCHANGE_FAILED = 3,
    AP_MSG_LOGIN_OK = 4,
    AP_MSG_LOGIN_FAILED = 5,
    AP_MSG_COMMAND_OK = 6,
    AP_MSG_COMMAND_FAILED = 7,
    AP_MSG_ENCRYPTION_OK = 8,
    AP_MSG_ENCRYPTION_FAILED = 9,
    AP_MSG_PING_OK = 10,
    AP_MSG_MESSAGE = 11,
    AP_MSG_ERROR = 12,
};

enum
{
    CMD_BROADCAST = 1,
    CMD_CLOSE_SERVER = 2,
    CMD_PAY_HOUSES = 3,
    //CMD_OPEN_SERVER = 4,
    CMD_SHUTDOWN_SERVER = 5,
    //CMD_RELOAD_SCRIPTS = 6,
    //CMD_PLAYER_INFO = 7,
    //CMD_GETONLINE = 8,
    CMD_KICK = 9,
    //CMD_BAN_MANAGER = 10,
    //CMD_SERVER_INFO = 11,
    //CMD_GETHOUSE = 12,
    CMD_SAVE_SERVER = 13,
    CMD_SEND_MAIL = 14,
    CMD_SHALLOW_SAVE_SERVER = 15,
    CMD_RELATIONAL_SAVE_SERVER = 16,
};

enum
{
    REQUIRE_LOGIN = 1,
    REQUIRE_ENCRYPTION = 2,
};

enum
{
    ENCRYPTION_RSA1024XTEA = 1,
};

class ProtocolAdmin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0xFE };
    enum { UseChecksum = false };
private:
    std::string password_;
    std::string host_;
    uint16_t port_;
    std::string serverString_;
    uint16_t security_;
    uint32_t options_;

    bool firstRecv_;
    void SendLoginPacket();
    void ParseMessage(const std::shared_ptr<InputMessage>& message);
    void ParseMessageHello(const std::shared_ptr<InputMessage>& message);
    void DoLogin();
    void SetupEncryption();
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
    void OnError(const asio::error_code& err) override;
public:
    ProtocolAdmin();
    ~ProtocolAdmin();

    void Login(const std::string& host, uint16_t port, const std::string& password);

    const std::string& GetServerString() const { return serverString_; }
    uint16_t GetSecurity() const { return security_; }
    uint32_t GetOptions() const { return options_; }
};

