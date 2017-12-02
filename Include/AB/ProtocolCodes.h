#pragma once

#include <stdint.h>

namespace AB {

enum ProtocolIds : uint8_t
{
    ProtocolLoginId = 0x01,
    ProtocolAdminId = 0xFE,
    ProtocolStatusId = 0xFF
};

namespace Errors {

enum ErrorCodes : uint8_t
{
    IPBanned = 0x01,
    TooManyConnectionsFromThisIP = 0x02,
    InvalidAccountName = 0x03,
    InvalidPassword = 0x04,
    NamePasswordMismatch = 0x05,
    AlreadyLoggedIn = 0x06,
    ErrorLoadingCharacter = 0x07,
    AccountBanned = 0x08,
};

}

namespace LoginProtocol {

enum LoginProtocolCodes : uint8_t
{
    LoginError = 0x01,
    CharacterList = 0x02,                   // Returns character list
};

}

namespace GameProtocol {

enum GameProtocolCodes : uint8_t
{
    NoError = 0x00,
    Error = 0x01,

    GameEnter = 0x02,
    GameUpdate = 0x03,
    GamePong = 0x04,
};

enum GamePacketTypes : uint8_t
{
    PacketTypePing = 0x01,

    PacketTypeLogout = 0x14,

    PacketTypeMoveNorth = 0x65,
    PacketTypeMoveNorthEast = 0x66,
    PacketTypeMoveEast = 0x67,
    PacketTypeMoveSouthEast = 0x68,
    PacketTypeMoveSouth = 0x69,
    PacketTypeMoveSouthWest = 0x70,
    PacketTypeMoveWest = 0x71,
    PacketTypeMoveNorthWest = 0x72,
};

}

}
