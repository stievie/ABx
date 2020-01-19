#pragma once

#include <stdint.h>

namespace AB
{

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
    0x76924dc0
};

// Packet types sent from the client to the server
#define ENUMERATE_CLIENT_PACKET_CODES                    \
    ENUMERATE_CLIENT_PACKET_CODE(Ping)                   \
    ENUMERATE_CLIENT_PACKET_CODE(Logout)                 \
    ENUMERATE_CLIENT_PACKET_CODE(ChangeMap)              \
    ENUMERATE_CLIENT_PACKET_CODE(SetOnlineStatus)        \
    ENUMERATE_CLIENT_PACKET_CODE(GetPlayerInfoByAccount) \
    ENUMERATE_CLIENT_PACKET_CODE(GetPlayerInfoByName)    \
    ENUMERATE_CLIENT_PACKET_CODE(SendMail)               \
    ENUMERATE_CLIENT_PACKET_CODE(GetMailHeaders)         \
    ENUMERATE_CLIENT_PACKET_CODE(GetMail)                \
    ENUMERATE_CLIENT_PACKET_CODE(DeleteMail)             \
    ENUMERATE_CLIENT_PACKET_CODE(Move)                   \
    ENUMERATE_CLIENT_PACKET_CODE(Turn)                   \
    ENUMERATE_CLIENT_PACKET_CODE(SetDirection)           \
    ENUMERATE_CLIENT_PACKET_CODE(Goto)                   \
    ENUMERATE_CLIENT_PACKET_CODE(Follow)                 \
    ENUMERATE_CLIENT_PACKET_CODE(SetState)               \
    ENUMERATE_CLIENT_PACKET_CODE(UseSkill)               \
    ENUMERATE_CLIENT_PACKET_CODE(Attack)                 \
    ENUMERATE_CLIENT_PACKET_CODE(Cancel)                 \
    ENUMERATE_CLIENT_PACKET_CODE(Select)                 \
    ENUMERATE_CLIENT_PACKET_CODE(ClickObject)            \
    ENUMERATE_CLIENT_PACKET_CODE(Command)                \
    ENUMERATE_CLIENT_PACKET_CODE(Queue)                  \
    ENUMERATE_CLIENT_PACKET_CODE(Unqueue)                \
    ENUMERATE_CLIENT_PACKET_CODE(PartyInvitePlayer)      \
    ENUMERATE_CLIENT_PACKET_CODE(PartyKickPlayer)        \
    ENUMERATE_CLIENT_PACKET_CODE(PartyLeave)             \
    ENUMERATE_CLIENT_PACKET_CODE(PartyAcceptInvite)      \
    ENUMERATE_CLIENT_PACKET_CODE(PartyRejectInvite)      \
    ENUMERATE_CLIENT_PACKET_CODE(GetPartyMembers)        \
    ENUMERATE_CLIENT_PACKET_CODE(SetSecondaryProfession) \
    ENUMERATE_CLIENT_PACKET_CODE(SetAttributeValue)      \
    ENUMERATE_CLIENT_PACKET_CODE(EquipSkill)             \
    ENUMERATE_CLIENT_PACKET_CODE(GetInventory)           \
    ENUMERATE_CLIENT_PACKET_CODE(InventoryDestroyItem)   \
    ENUMERATE_CLIENT_PACKET_CODE(InventoryDropItem)      \
    ENUMERATE_CLIENT_PACKET_CODE(InventoryStoreInChest)  \
    ENUMERATE_CLIENT_PACKET_CODE(GetChest)               \
    ENUMERATE_CLIENT_PACKET_CODE(ChestDestroyItem)       \
    ENUMERATE_CLIENT_PACKET_CODE(GetFriendList)          \
    ENUMERATE_CLIENT_PACKET_CODE(AddFriend)              \
    ENUMERATE_CLIENT_PACKET_CODE(RemoveFriend)           \
    ENUMERATE_CLIENT_PACKET_CODE(RenameFriend)           \
    ENUMERATE_CLIENT_PACKET_CODE(GetGuildInfo)           \
    ENUMERATE_CLIENT_PACKET_CODE(GetGuildMembers)

// Packet types sent from the server to the client
#define ENUMERATE_SERVER_PACKET_CODES                         \
    ENUMERATE_SERVER_PACKET_CODE(NoError)                     \
    ENUMERATE_SERVER_PACKET_CODE(Error)                       \
    ENUMERATE_SERVER_PACKET_CODE(KeyExchange)                 \
    ENUMERATE_SERVER_PACKET_CODE(ServerJoined)                \
    ENUMERATE_SERVER_PACKET_CODE(ServerLeft)                  \
    ENUMERATE_SERVER_PACKET_CODE(ServerMessage)               \
    ENUMERATE_SERVER_PACKET_CODE(ChatMessage)                 \
    ENUMERATE_SERVER_PACKET_CODE(MailHeaders)                 \
    ENUMERATE_SERVER_PACKET_CODE(MailComplete)                \
    ENUMERATE_SERVER_PACKET_CODE(ChangeInstance)              \
    ENUMERATE_SERVER_PACKET_CODE(PlayerError)                 \
    ENUMERATE_SERVER_PACKET_CODE(PlayerAutoRun)               \
    ENUMERATE_SERVER_PACKET_CODE(GameStart)                   \
    ENUMERATE_SERVER_PACKET_CODE(GameEnter)                   \
    ENUMERATE_SERVER_PACKET_CODE(GameUpdate)                  \
    ENUMERATE_SERVER_PACKET_CODE(GamePong)                    \
    ENUMERATE_SERVER_PACKET_CODE(GameSpawnObjectExisting)     \
    ENUMERATE_SERVER_PACKET_CODE(GameSpawnObject)             \
    ENUMERATE_SERVER_PACKET_CODE(GameLeaveObject)             \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectPositionChange)    \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectRotationChange)    \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectSelectTarget)      \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectStateChange)       \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectMoveSpeedChange)   \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectResourceChange)    \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectUseSkill)          \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectEndUseSkill)       \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectSkillFailure)      \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectAttackFailure)     \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectEffectAdded)       \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectEffectRemoved)     \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectDamaged)           \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectHealed)            \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectProgress)          \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectPingTarget)        \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectDropItem)          \
    ENUMERATE_SERVER_PACKET_CODE(GameObjectSetPosition)       \
    ENUMERATE_SERVER_PACKET_CODE(PartyPlayerInvited)          \
    ENUMERATE_SERVER_PACKET_CODE(PartyPlayerRemoved)          \
    ENUMERATE_SERVER_PACKET_CODE(PartyPlayerAdded)            \
    ENUMERATE_SERVER_PACKET_CODE(PartyInviteRemoved)          \
    ENUMERATE_SERVER_PACKET_CODE(PartyInfoMembers)            \
    ENUMERATE_SERVER_PACKET_CODE(PartyResigned)               \
    ENUMERATE_SERVER_PACKET_CODE(PartyDefeated)               \
    ENUMERATE_SERVER_PACKET_CODE(InventoryContent)            \
    ENUMERATE_SERVER_PACKET_CODE(InventoryItemUpdate)         \
    ENUMERATE_SERVER_PACKET_CODE(InventoryItemDelete)         \
    ENUMERATE_SERVER_PACKET_CODE(ChestContent)                \
    ENUMERATE_SERVER_PACKET_CODE(ChestItemUpdate)             \
    ENUMERATE_SERVER_PACKET_CODE(ChestItemDelete)             \
    ENUMERATE_SERVER_PACKET_CODE(FriendList)                  \
    ENUMERATE_SERVER_PACKET_CODE(FriendAdded)                 \
    ENUMERATE_SERVER_PACKET_CODE(FriendRemoved)               \
    ENUMERATE_SERVER_PACKET_CODE(FriendRenamed)               \
    ENUMERATE_SERVER_PACKET_CODE(GuildInfo)                   \
    ENUMERATE_SERVER_PACKET_CODE(GuildMemberList)             \
    ENUMERATE_SERVER_PACKET_CODE(DialogTrigger)               \
    ENUMERATE_SERVER_PACKET_CODE(QuestSelectionDialogTrigger) \
    ENUMERATE_SERVER_PACKET_CODE(QuestDialogTrigger)          \
    ENUMERATE_SERVER_PACKET_CODE(QuestNpcHasQuest)            \
    ENUMERATE_SERVER_PACKET_CODE(QuestDeleted)                \
    ENUMERATE_SERVER_PACKET_CODE(QuestRewarded)               \
    ENUMERATE_SERVER_PACKET_CODE(PlayerInfo)                  \
    ENUMERATE_SERVER_PACKET_CODE(PlayerSetAttributeValue)     \
    ENUMERATE_SERVER_PACKET_CODE(PlayerSetSecProfession)

namespace Errors
{

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
    TokenAuthFailure = 0x19,
    AccountKeyAlreadyAdded = 0x20,

    ErrorException = 0xff
};

}

namespace LoginProtocol
{

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
    CharacterList = 0x02, // Returns character list
    KeyExchange = 0x03,
    CreateAccountError = 0x04,
    CreateAccountSuccess = 0x05,
    CreatePlayerError = 0x06,
    CreatePlayerSuccess = 0x07,
    DeletePlayerError = 0x08,
    DeletePlayerSuccess = 0x09,
    AddAccountKeySuccess = 0x0a,
    AddAccountKeyError = 0x0b,
    OutpostList = 0x0c, // List of maps
    ServerList = 0x0d,  // List of running game server
};

}

namespace GameProtocol
{

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
    ServerMessageTypeRoll, // /roll result
    ServerMessageTypeAge,  // /age
    ServerMessageTypeHp,   // /hp
    ServerMessageTypeXp,   // /xp
    ServerMessageTypePos,  // /pos
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

    ServerMessageTypePlayerNotFound,

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

/// Packets sent by the server
enum class ServerPacketType : uint8_t
{
    __First = 0, // Let's start with 1
#define ENUMERATE_SERVER_PACKET_CODE(v) v,
    ENUMERATE_SERVER_PACKET_CODES
#undef ENUMERATE_SERVER_PACKET_CODE
    __Last = 0xFF
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
    ResourceTypeMorale,
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
    SkillErrorNotAppropriate = 255, // not an error but does not make sense to use it
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
    PlayerErrorInventoryFull
};

enum GameObjectType : uint8_t
{
    ObjectTypeUnknown = 0,
    ObjectTypeStatic,       // GameObject
    ObjectTypeTerrainPatch, // GameObject
    // -- Bellow all objects are sent to player when they spawn ----------------
    ObjectTypeSentToPlayer, // Not an actual object type, all bellow is sent to the player
    ObjectTypeItemDrop,     // GameObject
    ObjectTypeAreaOfEffect, // GameObject: Area that affects actors in it, e.g. a well
    // Bellow are all Actors
    ObjectTypeProjectile, // Actor
    ObjectTypeNpc,
    ObjectTypePlayer, // Human player
};

enum PlayerInfoFields : uint32_t
{
    PlayerInfoFieldName = 1,
    PlayerInfoFieldCurrentName = 1 << 1,
    PlayerInfoFieldCurrentMap = 1 << 2,
    PlayerInfoFieldOnlineStatus = 1 << 3,
    PlayerInfoFieldRelation = 1 << 4,
    PlayerInfoFieldGuildGuid = 1 << 5,
    PlayerInfoFieldGuildRole = 1 << 6,
    PlayerInfoFieldGuildInviteName = 1 << 7,
    PlayerInfoFieldGuildInvited = 1 << 8,
    PlayerInfoFieldGuildJoined = 1 << 9,
    PlayerInfoFieldGuildExpires = 1 << 10
};
static constexpr uint32_t PlayerInfoFieldsAll = 0xFFFFFFFF;

/// What fields are sent with GameSpawnObjectExisting and GameSpawnObject the message
enum ObjectSpawnFields : uint32_t
{
    ObjectSpawnFieldPos = 1,
    ObjectSpawnFieldRot = 1 << 1,
    ObjectSpawnFieldScale = 1 << 2,
    ObjectSpawnFieldUndestroyable = 1 << 3,
    ObjectSpawnFieldSelectable = 1 << 4,
    ObjectSpawnFieldState = 1 << 5,
    ObjectSpawnFieldSpeed = 1 << 6,
    ObjectSpawnFieldGroupId = 1 << 7,
    ObjectSpawnFieldGroupPos = 1 << 8,
};

/// What fields contains the data PropStream
enum ObjectSpawnDataFields : uint32_t
{
    ObjectSpawnDataFieldName = 1,
    ObjectSpawnDataFieldLevel = 1 << 1,
    ObjectSpawnDataFieldSex = 1 << 2,
    ObjectSpawnDataFieldProf = 1 << 3,
    ObjectSpawnDataFieldModelIndex = 1 << 4,
    ObjectSpawnDataFieldSkills = 1 << 5,
};

/// Packets sent by the client
enum class ClientPacketTypes : uint8_t
{
    __First = 0, // Let's start with 1
#define ENUMERATE_CLIENT_PACKET_CODE(v) v,
    ENUMERATE_CLIENT_PACKET_CODES
#undef ENUMERATE_CLIENT_PACKET_CODE
};

enum class CommandType : uint8_t
{
    Unknown = 0,
    // Chat
    ChatGeneral = 1, // /a <message>
    ChatGuild,       // /g <message>
    ChatParty,       // /p <message>
    ChatTrade,       // /trade <message>
    ChatWhisper,     // /w <name>, <message>

    Resign, // /resign
    Stuck,  // /stuck

    // Info
    Age,    // /age
    Deaths, // /deaths
    Health, // /hp
    Xp,     // /xp
    Pos,    // /pos show position
    // Emotes
    Roll,   // /roll <number>
    Sit,    // /sit
    Stand,  // /sit -> Idle
    Cry,    // /cry
    Taunt,  // /taunt
    Ponder, // /ponder
    Wave,   // /wave
    Laugh,  // /laugh
    // Admin/GM
    Admin = 50,
    EnterMap = Admin, // /entermap <name>
    EnterInstance,               // /enterinstance <uuid>
    Instances,                   // /instances
    Die,                         // /die (only GM)
    GodMode,                     // /gm  Toggle god mode
    GotoPlayer,                  // /gotoplayer <name>
    GMInfo,                      // /gminfo <message>  GM info massage

    // Internal
    Internal = 100,
    Help = Internal, // /help
    Ip,                         // /ip
    ServerId,                   // /id
    PrefPath,                   // /prefpath
    Quit,
};

}

}
