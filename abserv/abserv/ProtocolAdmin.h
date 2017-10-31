#pragma once

// -> server
// command(1 byte) | size(2 bytes) | parameters(size bytes)
// commands:
//  login
//    password(string)
//  encryption
//    encryption type(1 byte)
//      RSA1024+XTEA
//        :128 bytes encrypted using 1024 bytes public key
//        16 bytes XTEA key
//  key-exchange
//    public_key_type(1 byte)
//      RSA1024+XTEA
//  command
//    command + paramters(string)
//  no_operation/ping
//    nothing
//
// <- server
// ret-code(1 byte)| size(2 bytes) | parameters(size bytes)
// ret-codes:
//  hello
//    server_version(4 bytes)
//    server_string(string)
//    security_policy(2 bytes flags)
//      required_login
//      required_encryption
//    accepted_encryptions(4 bytes flags)
//      RSA1024+XTEA
//  key-exchange-ok
//    public_key_type(1 byte)
//      RSA1024+XTEA
//        :128 bytes public key modulus
//  key-exchange-failed
//    reason(string)
//  login-ok
//    nothing
//  login-failed
//    reason(string)
//  command-ok
//    command result(string)
//  command-failed
//    reason(string)
//  encryption-ok
//    nothing
//  encryption-failed
//    reason(string)
//  no_operation-ok
//    nothing
//  message
//    message(string)
//  error
//    message(string)
//

#include <memory>
#include "Protocol.h"
#include "Connection.h"
#include <stdint.h>

namespace Net {

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
    AP_MSG_ERROR = 12
};

enum
{
    CMD_BROADCAST = 1,
    CMD_CLOSE_SERVER = 2,
    CMD_PAY_HOUSES = 3,
    CMD_OPEN_SERVER = 4,
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
    CMD_RELATIONAL_SAVE_SERVER = 16
};

enum
{
    REQUIRE_LOGIN = 1,
    REQUIRE_ENCRYPTION = 2
};

enum
{
    ENCRYPTION_RSA1024XTEA = 1
};

class ProtocolAdmin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0xFE };
    enum { UseChecksum = false };
    static const char* ProtocolName() { return "Admin Protocol"; };
private:
    enum ConnectionState
    {
        NotConnected,
        EncryptionToSet,
        EncryptionOK,
        NotloggedIn,
        LoggedIn
    };
    ConnectionState state_;
    uint32_t loginTries_;
    time_t lastCommand_;
    time_t startTime_;
    bool requireLogin_;
    bool requireEncryption_;
    void HandleMsgLogin(NetworkMessage& message, OutputMessage* output);
    void HandleMsgEncryption(NetworkMessage& message, OutputMessage* output);
    void HandleMsgKeyExchange(NetworkMessage& message, OutputMessage* output);
    void HandleMsgPing(NetworkMessage& message, OutputMessage* output);
    void HandleMsgCommand(NetworkMessage& message, OutputMessage* output);
    void CommandKickPlayer(const std::string& name);
    void CommandShutdownServer();
public:
    explicit ProtocolAdmin(std::shared_ptr<Connection> connection);

    void OnRecvFirstMessage(NetworkMessage& msg) override;
    void ParsePacket(NetworkMessage& message) override;

    bool AllowIP(uint32_t clientIP);
    uint16_t GetProtocolPolicy() const
    {
        uint16_t policy = 0;
        if (requireLogin_)
            policy |= REQUIRE_LOGIN;
        if (requireEncryption_)
            policy |= REQUIRE_ENCRYPTION;
        return policy;
    }
    uint32_t GetProtocolOptions() const
    {
        uint32_t options = 0;
        if (requireEncryption_)
        {

        }
        return options;
    }
};

}
