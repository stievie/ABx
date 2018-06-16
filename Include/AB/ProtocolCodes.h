#pragma once

#include <stdint.h>

namespace AB {

/// Increase whenever the protocol changes
static constexpr uint16_t PROTOCOL_VERSION = 1;

static constexpr uint16_t CLIENT_OS_WIN = 1;
static constexpr uint16_t CLIENT_OS_LINUX = 2;
static constexpr uint16_t CLIENT_OS_MAC = 3;
#ifdef _WIN32
static constexpr uint16_t CLIENT_OS_CURRENT = CLIENT_OS_WIN;
#endif // _WIN32

enum ProtocolIds : uint8_t
{
    ProtocolLoginId = 0x01,
    ProtocolAdminId = 0xFE,
    ProtocolStatusId = 0xFF
};

#define ENABLE_GAME_ENCRYTION true

const uint32_t ENC_KEY[4] = {
    0xd705d09f,
    0x72a8a08c,
    0xecabe20b,
    0x76924dc0};

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
    WrongProtocolVersion = 0x0a,
    InvalidEmail = 0x0b,
    InvalidAccountKey = 0x0c,
    UnknownError = 0x0d,
    AccountNameExists = 0x0e,
    InvalidCharacterName = 0x0f,
    InvalidProfession = 0x10,
    PlayerNameExists = 0x11,
    InvalidAccount = 0x12,
    InvalidPlayerSex = 0x13,
    InvalidCharacter = 0x14,
    InvalidCharactersInString = 0x15,
    NoMoreCharSlots = 0x16,
    InvalidGame = 0x17,
    AllServersFull = 0x18,
};

}

namespace LoginProtocol {

enum LoginPacketTypes : uint8_t
{
    LoginLogin = 0x01,
    LoginCreateAccount = 0x03,
    LoginCreateCharacter = 0x04,
    LoginDeleteCharacter = 0x05,
    LoginAddAccountKey = 0x06,
    LoginGetOutposts = 0x07,
    LoginGetGameServers = 0x08,
};

/// Returned by the server
enum LoginProtocolCodes : uint8_t
{
    LoginError = 0x01,
    CharacterList = 0x02,                   // Returns character list
    KeyExchange = 0x03,
    CreateAccountError = 0x04,
    CreateAccountSuccess = 0x05,
    CreatePlayerError = 0x06,
    CreatePlayerSuccess = 0x07,
    DeletePlayerError = 0x08,
    DeletePlayerSuccess = 0x09,
    AddAccountKeySuccess = 0x0a,
    AddAccountKeyError = 0x0b,
    OutpostList = 0x0c,                       // List of maps
    ServerList = 0x0d,                        // List of running game server
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

enum TurnDirection : uint8_t
{
    TurnDirectionNone = 0,
    TurnDirectionLeft = 1,
    TurnDirectionRight = 1 << 1
};

enum CreatureState : uint8_t
{
    CreatureStateIdle = 0,
    CreatureStateMoving = 1,
    CreatureStateUsingSkill = 3,
    CreatureStateAttacking = 4,
    CreatureStateEmote = 5,
};

enum ServerMessageType : uint8_t
{
    ServerMessageTypeUnknown = 0,
    ServerMessageTypeInfo,
    ServerMessageTypeRoll,                    // /roll result
    ServerMessageTypeAge,                     // /age
    ServerMessageTypePlayerNotOnline,
    ServerMessageTypePlayerGotMessage,
    ServerMessageTypeNewMail,
    ServerMessageTypeMailSent,
    ServerMessageTypeMailNotSent,
    ServerMessageTypeMailboxFull,
    ServerMessageTypeMailDeleted,
    ServerMessageTypeServerId,

    ServerMessageTypeUnknownCommand = 0xff
};

enum ChatMessageChannel : uint8_t
{
    ChatChannelUnknown,
    ChatChannelGeneral,
    ChatChannelGuild,
    ChatChannelParty,
    ChatChannelTrade,
    ChatChannelWhisper,
};

enum GameProtocolCodes : uint8_t
{
    NoError = 0x00,
    Error = 0x01,

    WelcomeMessage,
    ServerMessage,
    ChatMessage,
    // Mail
    MailHeaders,
    MailComplete,

    GameStart,                         // Start tick
    GameEnter,
    GameUpdate,
    GamePong,
    // An object spawned
    GameSpawnObjectExisting,
    GameSpawnObject,
    GameLeaveObject,
    // Object Update
    GameObjectPositionChange,
    GameObjectRotationChange,
    GameObjectSelectTarget,
    GameObjectStateChange,
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

    PacketTypeLogout,

    // Mail
    PacketTypeGetMailHeaders,
    PacketTypeGetMail,
    PacketTypeDeleteMail,

    // Move
    PacketTypeMove,
    PacketTypeTurn,
    PacketTypeSetDirection,
    PacketTypeGoto,                  // Goto point
    PacketTypeFollow,                // Follow object

    // Skills
    PacketTypeUseSkill,
    PacketTypeCancelSkill,
    // Attack
    PacketTypeAttack,
    PacketTypeCancelAttack,
    // Select
    PacketTypeSelect,
    // Command
    PacketTypeCommand,

    // Party
    PacketTypePartyInvitePlayer,
    PacketTypePartyKickPlayer,
    PacketTypePartyLeave,
    PacketTypePartyRequestJoin,
};

enum CommandTypes : uint8_t
{
    CommandTypeUnknown = 0,
    // Chat
    CommandTypeChatGeneral = 1,      // /a <message>
    CommandTypeChatGuild,            // /g <message>
    CommandTypeChatParty,            // /p <message>
    CommandTypeChatTrade,            // /trade <message>
    CommandTypeChatWhisper,          // /w <name>, <message>
    // Mail
    CommandTypeMailSend,             // /mail <name>, <message>
    CommandTypeMailInbox,            // /inbox
    CommandTypeMailRead,             // /read <index>
    CommandTypeMailDelete,           // /delete <index>

    // Other
    CommandTypeRoll,                 // /roll <number>
    // Info
    CommandTypeAge,                  // /age
    CommandTypeDeaths,               // /deaths
    CommandTypeHealth,               // /hp
    // Internal
    CommandTypeHelp,                 // /help
    CommandTypeIp,                   // /ip
    CommandTypeServerId              // /id
};

}

}
