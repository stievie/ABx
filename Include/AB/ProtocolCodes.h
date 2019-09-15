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
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
static constexpr uint16_t CLIENT_OS_CURRENT = CLIENT_OS_LINUX;
#elif (defined(__APPLE__) && defined(__MACH__))
static constexpr uint16_t CLIENT_OS_CURRENT = CLIENT_OS_MAC;
#endif

enum ProtocolIds : uint8_t
{
    ProtocolLoginId = 0x01,
    ProtocolAdminId = 0xFE,
    ProtocolStatusId = 0xFF
};

#define ENABLE_GAME_ENCRYTION true
// TODO:
#define ENABLE_GAME_COMPRESSION false

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

    ErrorException = 0xff
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
    CreatureStateUnknown = 0,
    CreatureStateIdle = 1,
    CreatureStateMoving = 2,
    CreatureStateUsingSkill = 3,
    CreatureStateAttacking = 4,
    CreatureStateKnockedDown = 5,
    CreatureStateEmote = 6,
    CreatureStateEmoteSit = 7,

    // Emotes ------------------------------------------------------------------
    CreatureStateEmoteStart,
    CreatureStateEmoteCry,
    CreatureStateEmoteTaunt,
    CreatureStateEmotePonder,
    CreatureStateEmoteWave,
    CreatureStateEmoteLaugh,
    CreatureStateEmoteEnd,
    // /Emotes -----------------------------------------------------------------

    // Logic actors states -----------------------------------------------------
    CreatureStateChestClosed,
    CreatureStateChestOpen,
    CreatureStateTriggered,
    // /Logic actors states ----------------------------------------------------


    CreatureStateDead = 255,
};

enum ServerMessageType : uint8_t
{
    ServerMessageTypeUnknown = 0,
    ServerMessageTypeInfo,
    ServerMessageTypeRoll,                    // /roll result
    ServerMessageTypeAge,                     // /age
    ServerMessageTypeHp,                      // /hp
    ServerMessageTypeXp,                      // /xp
    ServerMessageTypePos,                     // /pos
    ServerMessageTypePlayerNotOnline,
    ServerMessageTypePlayerGotMessage,
    ServerMessageTypeNewMail,
    ServerMessageTypeMailSent,
    ServerMessageTypeMailNotSent,
    ServerMessageTypeMailboxFull,
    ServerMessageTypeMailDeleted,
    ServerMessageTypeServerId,
    ServerMessageTypePlayerResigned,
    ServerMessageTypePlayerQueued,
    ServerMessageTypePlayerUnqueued,

    ServerMessageTypeInstances,
    ServerMessageTypeGMInfo,

    ServerMessageTypeUnknownCommand = 0xff
};

enum ChatMessageChannel : uint8_t
{
    ChatChannelUnknown = 0,
    ChatChannelGeneral,
    ChatChannelGuild,
    ChatChannelParty,
    ChatChannelAllies,
    ChatChannelTrade,
    ChatChannelWhisper,
};

enum GameProtocolCodes : uint8_t
{
    NoError = 0x00,
    Error = 0x01,
    KeyExchange,

    ServerJoined,
    ServerLeft,
    ServerMessage,
    ChatMessage,
    // Mail
    MailHeaders,
    MailComplete,

    ChangeInstance,

    PlayerError,                       // Some GameError code
    PlayerAutoRun,                     // Server is controlling the player, client must stop client prediction

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
    GameObjectMoveSpeedChange,
    GameObjectResourceChange,
    GameObjectUseSkill,           // Actor successfully activated a skill, returns resources needed etc.
    GameObjectEndUseSkill,        // Actor successfully finished a skill, returns recharge.
    GameObjectSkillFailure,       // Failed activating a skill, returns some error code
    GameObjectAttackFailure,
    GameObjectEffectAdded,
    GameObjectEffectRemoved,
    GameObjectDamaged,            // Object got damage
    GameObjectHealed,
    GameObjectProgress,           // The object progressed, see ObjectProgressType
    GameObjectPingTarget,
    GameObjectDropItem,           // An object dropped an item
    GameObjectSetPosition,        // Force set position
    // Party
    PartyPlayerInvited,
    PartyPlayerRemoved,
    PartyPlayerAdded,
    PartyInviteRemoved,
    PartyInfoMembers,
    PartyResigned,
    PartyDefeated,
    // Inventory
    InventoryContent,             // All inventory
    InventoryItemUpdate,
    InventoryItemDelete,
    // Chest
    ChestContent,
    ChestItemUpdate,
    ChestItemDelete,
    // Friend list
    FriendListAll,
    // Guild
    GuildMembersAll,

    DialogTrigger,                // Show a dialog
    PlayerLoggedIn,                    // Notification sent to friends/guild members when a player logged in
    PlayerLoggedOut,                   // Notification sent to friends/guild members when a player logged out

    CodeLast = 0xFF
};

enum ObjectCallType : uint8_t
{
    ObjectCallTypeNone = 0,
    ObjectCallTypeFollow,
    ObjectCallTypeAttack,
    ObjectCallTypeUseSkill
};

enum ObjectProgressType : uint8_t
{
    ObjectProgressXPIncreased,
    ObjectProgressGotSkillPoint,
    ObjectProgressLevelAdvance,
    ObjectProgressAttribPointsGain,
};

enum ResourceType : uint8_t
{
    ResourceTypeHealth = 1,
    ResourceTypeEnergy,
    ResourceTypeAdrenaline,
    ResourceTypeOvercast,
    ResourceTypeHealthRegen,
    ResourceTypeEnergyRegen,
    ResourceTypeMaxHealth,
    ResourceTypeMaxEnergy,
};

enum SkillError : uint8_t
{
    SkillErrorNone = 0,
    SkillErrorInvalidSkill,
    SkillErrorInvalidTarget,
    SkillErrorOutOfRange,
    SkillErrorNoEnergy,
    SkillErrorNoAdrenaline,
    SkillErrorRecharging,
    SkillErrorTargetUndestroyable,
    SkillErrorCannotUseSkill,
};

enum AttackError : uint8_t
{
    AttackErrorNone = 0,
    AttackErrorInvalidTarget,
    AttackErrorTargetUndestroyable,
    AttackErrorInterrupted,
    AttackErrorNoTarget,
    AttackErrorTargetObstructed,
    AttackErrorTargetDodge,
    AttackErrorTargetMissed,
};

enum PlayerErrorValue : uint8_t
{
    PlayerErrorNone = 0,
    PlayerErrorInventoryFull,
};

enum GameObjectType : uint8_t
{
    ObjectTypeUnknown = 0,
    ObjectTypeStatic,
    ObjectTypeTerrainPatch,
    // -- Bellow all objects are sent to player when they spawn ----------------
    ObjectTypeSentToPlayer,          // Not an actual object type, all bellow is sent to the player
    ObjectTypeNpc,
    ObjectTypePlayer,                // Human player
    ObjectTypeProjectile,
    ObjectTypeAreaOfEffect,          // Area that affects actors in it, e.g. a well
    ObjectTypeItemDrop,
};

enum GamePacketTypes : uint8_t
{
    PacketTypePing = 0x01,

    PacketTypeLogout,
    PacketTypeChangeMap,

    // Mail
    PacketTypeSendMail,
    PacketTypeGetMailHeaders,
    PacketTypeGetMail,
    PacketTypeDeleteMail,

    // Move
    PacketTypeMove,
    PacketTypeTurn,
    PacketTypeSetDirection,
    PacketTypeGoto,                  // Goto point
    PacketTypeFollow,                // Follow object
    PacketTypeSetState,

    // Skills
    PacketTypeUseSkill,
    // Attack
    PacketTypeAttack,
    PacketTypeCancel,                // Cancel skill/attack
    // Select
    PacketTypeSelect,
    PacketTypeClickObject,
    // Command
    PacketTypeCommand,
    // Queue
    PacketTypeQueue,
    PacketTypeUnqueue,

    // Party
    PacketTypePartyInvitePlayer,
    PacketTypePartyKickPlayer,
    PacketTypePartyLeave,
    PacketTypePartyAcceptInvite,
    PacketTypePartyRejectInvite,
    PacektTypeGetPartyMembers,

    // Inventory
    PacketTypeGetInventory,
    PacketTypeInventoryDestroyItem,
    PacketTypeInventoryDropItem,
    PacketTypeInventoryStoreInChest,
    // Chest
    PacketTypeGetChest,
    PacketTypeChestDestroyItem,
    // Friend list
    PacketTypeGetFriendList,
    // Guild
    PacketTypeGetGuildMembers,
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

    CommandTypeResign,               // /resign
    CommandTypeStuck,                // /stuck

    // Info
    CommandTypeAge,                  // /age
    CommandTypeDeaths,               // /deaths
    CommandTypeHealth,               // /hp
    CommandTypeXp,                   // /xp
    CommandTypePos,                  // /pos show position
    // Emotes
    CommandTypeRoll,                 // /roll <number>
    CommandTypeSit,                  // /sit
    CommandTypeStand,                // /sit -> Idle
    CommandTypeCry,                  // /cry
    CommandTypeTaunt,                // /taunt
    CommandTypePonder,               // /ponder
    CommandTypeWave,                 // /wave
    CommandTypeLaugh,                // /laugh
    // Admin/GM
    CommandTypeAdmin = 50,
    CommandTypeEnterMap = CommandTypeAdmin,             // /entermap <name>
    CommandTypeEnterInstance,        // /enterinstance <uuid>
    CommandTypeInstances,            // /instances
    CommandTypeDie,                  // /die (only GM)
    CommandTypeGodMode,              // /gm  Toggle god mode
    CommandTypeGotoPlayer,           // /gotoplayer <name>
    CommandTypeGMInfo,               // /gminfo <message>  GM info massage

    // Internal
    CommandTypeInternal = 100,
    CommandTypeHelp = CommandTypeInternal, // /help
    CommandTypeIp,       // /ip
    CommandTypeServerId, // /id
    CommandTypePrefPath, // /prefpath
    CommandTypeQuit,
};

}

}
