#pragma once

#include <stdint.h>

namespace AB {

/// Increase whenever the protocol changes
static constexpr uint16_t PROTOCOL_VERSION = 1;

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
    CannotEnterGame = 0x09,
    WrongProtocolVersion = 0x10,
};

}

namespace LoginProtocol {

enum LoginProtocolCodes : uint8_t
{
    LoginError = 0x01,
    CharacterList = 0x02,                   // Returns character list
    KeyExchange = 0x03,
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
    // An object spawned
    GameSpawnObjectExisting = 0x05,
    GameSpawnObject = 0x06,
    GameLeaveObject = 0x07,
};

enum GameObjectType : uint8_t
{
    ObjectTypeUnknown = 0x00,
    ObjectTypeStatic = 0x01,
    ObjectTypeNpc = 0x02,
    ObjectTypePlayer = 0x03,
    ObjectTypeProjectile = 0x04,
};

enum GamePacketTypes : uint8_t
{
    PacketTypePing = 0x01,

    PacketTypeLogout = 0x10,

    // Move
    PacketTypeMoveNorth = 0x20,
    PacketTypeMoveNorthEast = 0x21,
    PacketTypeMoveEast = 0x22,
    PacketTypeMoveSouthEast = 0x23,
    PacketTypeMoveSouth = 0x24,
    PacketTypeMoveSouthWest = 0x25,
    PacketTypeMoveWest = 0x26,
    PacketTypeMoveNorthWest = 0x28,

    // Skills
    PacketTypeUseSkill = 0x40,
    PacketTypeCancelSkill = 0x42,
    // Attack
    PacketTypeAttack = 0x50,
    PacketTypeCancelAttack = 0x51,
    // Select
    PacketTypeSelect = 0x60,
};

}

}
