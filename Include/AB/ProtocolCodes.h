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

/// Direction relative to current rotation
enum MoveDirection : uint8_t
{
    MoveDirectionNone = 0,
    MoveDirectionNorth = 1,
    MoveDirectionWest = 1 << 1,
    MoveDirectionSouth = 1 << 2,
    MoveDirectionEast = 1 << 3,
};

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
    // Object Update
    GameObjectPosUpdate = 0x08,
    GameObjectSelectTarget = 0x09,
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
    PacketTypeMove = 0x20,
    PacketTypeTurn = 0x21,

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
