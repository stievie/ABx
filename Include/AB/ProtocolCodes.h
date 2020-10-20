/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
#elif defined(__unix__) || defined(__unix)
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
    ENUMERATE_CLIENT_PACKET_CODE(SetState)               \
    ENUMERATE_CLIENT_PACKET_CODE(UseSkill)               \
    ENUMERATE_CLIENT_PACKET_CODE(Interact)               \
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
    ENUMERATE_CLIENT_PACKET_CODE(SetItemPos)             \
    ENUMERATE_CLIENT_PACKET_CODE(GetChest)               \
    ENUMERATE_CLIENT_PACKET_CODE(ChestDestroyItem)       \
    ENUMERATE_CLIENT_PACKET_CODE(WithdrawMoney)          \
    ENUMERATE_CLIENT_PACKET_CODE(DepositMoney)           \
    ENUMERATE_CLIENT_PACKET_CODE(GetFriendList)          \
    ENUMERATE_CLIENT_PACKET_CODE(AddFriend)              \
    ENUMERATE_CLIENT_PACKET_CODE(RemoveFriend)           \
    ENUMERATE_CLIENT_PACKET_CODE(RenameFriend)           \
    ENUMERATE_CLIENT_PACKET_CODE(GetGuildInfo)           \
    ENUMERATE_CLIENT_PACKET_CODE(GetGuildMembers)        \
    ENUMERATE_CLIENT_PACKET_CODE(LoadSkillTemplate)      \
    ENUMERATE_CLIENT_PACKET_CODE(TradeRequest)           \
    ENUMERATE_CLIENT_PACKET_CODE(TradeCancel)            \
    ENUMERATE_CLIENT_PACKET_CODE(TradeOffer)             \
    ENUMERATE_CLIENT_PACKET_CODE(TradeAccept)            \
    ENUMERATE_CLIENT_PACKET_CODE(SellItem)               \
    ENUMERATE_CLIENT_PACKET_CODE(BuyItem)                \
    ENUMERATE_CLIENT_PACKET_CODE(GetMerchantItems)       \
    ENUMERATE_CLIENT_PACKET_CODE(GetItemsPrice)          \
    ENUMERATE_CLIENT_PACKET_CODE(GetCraftsmanItems)      \
    ENUMERATE_CLIENT_PACKET_CODE(CraftItem)              \
    ENUMERATE_CLIENT_PACKET_CODE(SalvageItem)            \
    ENUMERATE_CLIENT_PACKET_CODE(PingPos)


// Packet types sent from the server to the client
#define ENUMERATE_SERVER_PACKET_CODES                         \
    ENUMERATE_SERVER_PACKET_CODE(ProtocolError)               \
    ENUMERATE_SERVER_PACKET_CODE(KeyExchange)                 \
    ENUMERATE_SERVER_PACKET_CODE(ServerJoined)                \
    ENUMERATE_SERVER_PACKET_CODE(ServerLeft)                  \
    ENUMERATE_SERVER_PACKET_CODE(ServerMessage)               \
    ENUMERATE_SERVER_PACKET_CODE(ChatMessage)                 \
    ENUMERATE_SERVER_PACKET_CODE(MailHeaders)                 \
    ENUMERATE_SERVER_PACKET_CODE(MailComplete)                \
    ENUMERATE_SERVER_PACKET_CODE(ChangeInstance)              \
    ENUMERATE_SERVER_PACKET_CODE(PlayerError)                 \
    ENUMERATE_SERVER_PACKET_CODE(PlayerAutorun)               \
    ENUMERATE_SERVER_PACKET_CODE(GameStart)                   \
    ENUMERATE_SERVER_PACKET_CODE(EnterWorld)                  \
    ENUMERATE_SERVER_PACKET_CODE(GameUpdate)                  \
    ENUMERATE_SERVER_PACKET_CODE(GamePong)                    \
    ENUMERATE_SERVER_PACKET_CODE(ObjectSpawnExisting)         \
    ENUMERATE_SERVER_PACKET_CODE(ObjectSpawn)                 \
    ENUMERATE_SERVER_PACKET_CODE(ObjectDespawn)               \
    ENUMERATE_SERVER_PACKET_CODE(ObjectPositionUpdate)        \
    ENUMERATE_SERVER_PACKET_CODE(ObjectRotationUpdate)        \
    ENUMERATE_SERVER_PACKET_CODE(ObjectTargetSelected)        \
    ENUMERATE_SERVER_PACKET_CODE(ObjectStateChanged)          \
    ENUMERATE_SERVER_PACKET_CODE(ObjectSpeedChanged)          \
    ENUMERATE_SERVER_PACKET_CODE(ObjectResourceChanged)       \
    ENUMERATE_SERVER_PACKET_CODE(ObjectUseSkill)              \
    ENUMERATE_SERVER_PACKET_CODE(ObjectSkillSuccess)          \
    ENUMERATE_SERVER_PACKET_CODE(ObjectSkillFailure)          \
    ENUMERATE_SERVER_PACKET_CODE(ObjectAttackFailure)         \
    ENUMERATE_SERVER_PACKET_CODE(ObjectEffectAdded)           \
    ENUMERATE_SERVER_PACKET_CODE(ObjectEffectRemoved)         \
    ENUMERATE_SERVER_PACKET_CODE(ObjectDamaged)               \
    ENUMERATE_SERVER_PACKET_CODE(ObjectHealed)                \
    ENUMERATE_SERVER_PACKET_CODE(ObjectProgress)              \
    ENUMERATE_SERVER_PACKET_CODE(ObjectPingTarget)            \
    ENUMERATE_SERVER_PACKET_CODE(ObjectDroppedItem)           \
    ENUMERATE_SERVER_PACKET_CODE(ObjectForcePosition)         \
    ENUMERATE_SERVER_PACKET_CODE(ObjectGroupMaskChanged)      \
    ENUMERATE_SERVER_PACKET_CODE(ObjectSetAttackSpeed)        \
    ENUMERATE_SERVER_PACKET_CODE(PartyPlayerInvited)          \
    ENUMERATE_SERVER_PACKET_CODE(PartyPlayerRemoved)          \
    ENUMERATE_SERVER_PACKET_CODE(PartyPlayerAdded)            \
    ENUMERATE_SERVER_PACKET_CODE(PartyInviteRemoved)          \
    ENUMERATE_SERVER_PACKET_CODE(PartyMembersInfo)            \
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
    ENUMERATE_SERVER_PACKET_CODE(NpcHasQuest)                 \
    ENUMERATE_SERVER_PACKET_CODE(QuestDeleted)                \
    ENUMERATE_SERVER_PACKET_CODE(QuestRewarded)               \
    ENUMERATE_SERVER_PACKET_CODE(PlayerInfo)                  \
    ENUMERATE_SERVER_PACKET_CODE(SetObjectAttributeValue)     \
    ENUMERATE_SERVER_PACKET_CODE(ObjectSecProfessionChanged)  \
    ENUMERATE_SERVER_PACKET_CODE(ObjectSetSkill)              \
    ENUMERATE_SERVER_PACKET_CODE(SkillTemplateLoaded)         \
    ENUMERATE_SERVER_PACKET_CODE(TradeDialogTrigger)          \
    ENUMERATE_SERVER_PACKET_CODE(TradeCancel)                 \
    ENUMERATE_SERVER_PACKET_CODE(TradeOffer)                  \
    ENUMERATE_SERVER_PACKET_CODE(TradeAccepted)               \
    ENUMERATE_SERVER_PACKET_CODE(MerchantItems)               \
    ENUMERATE_SERVER_PACKET_CODE(ItemPrice)                   \
    ENUMERATE_SERVER_PACKET_CODE(CraftsmanItems)              \
    ENUMERATE_SERVER_PACKET_CODE(DropTargetChanged)           \
    ENUMERATE_SERVER_PACKET_CODE(PositionPinged)

#define ENUMERATE_CREATURE_STATES          \
    ENUMERATE_CREATURE_STATE(Unknown)      \
    ENUMERATE_CREATURE_STATE(Idle)         \
    ENUMERATE_CREATURE_STATE(Moving)       \
    ENUMERATE_CREATURE_STATE(UsingSkill)   \
    ENUMERATE_CREATURE_STATE(Attacking)    \
    ENUMERATE_CREATURE_STATE(KnockedDown)  \
    ENUMERATE_CREATURE_STATE(Emote)        \
    ENUMERATE_CREATURE_STATE(EmoteSit)     \
    ENUMERATE_CREATURE_STATE(__EmoteStart) \
    ENUMERATE_CREATURE_STATE(EmoteCry)     \
    ENUMERATE_CREATURE_STATE(EmoteTaunt)   \
    ENUMERATE_CREATURE_STATE(EmotePonder)  \
    ENUMERATE_CREATURE_STATE(EmoteWave)    \
    ENUMERATE_CREATURE_STATE(EmoteLaugh)   \
    ENUMERATE_CREATURE_STATE(__EmoteEnd)   \
    ENUMERATE_CREATURE_STATE(ChestClosed)  \
    ENUMERATE_CREATURE_STATE(ChestOpen)    \
    ENUMERATE_CREATURE_STATE(Triggered)    \
    ENUMERATE_CREATURE_STATE(Dead)

#define ENUMERATE_SKILL_ERROR_CODES                                   \
    ENUMERATE_SKILL_ERROR_CODE(None)                        \
    ENUMERATE_SKILL_ERROR_CODE(InvalidSkill)                \
    ENUMERATE_SKILL_ERROR_CODE(InvalidTarget)               \
    ENUMERATE_SKILL_ERROR_CODE(OutOfRange)                  \
    ENUMERATE_SKILL_ERROR_CODE(NoEnergy)                    \
    ENUMERATE_SKILL_ERROR_CODE(NoAdrenaline)                \
    ENUMERATE_SKILL_ERROR_CODE(Recharging)                  \
    ENUMERATE_SKILL_ERROR_CODE(TargetUndestroyable)         \
    ENUMERATE_SKILL_ERROR_CODE(CannotUseSkill)              \
    ENUMERATE_SKILL_ERROR_CODE(NotAppropriate)
// SkillErrorNotAppropriate is not really an error, using the skill succeeds,
// but it doesn't make sense to use it.

enum class ErrorCodes : uint8_t
{
    NoError = 0,
    IPBanned,
    TooManyConnectionsFromThisIP,
    InvalidAccountName,
    InvalidPassword,
    NamePasswordMismatch,
    AlreadyLoggedIn,
    ErrorLoadingCharacter,
    AccountBanned,
    CannotEnterGame,
    WrongProtocolVersion,
    InvalidEmail,
    InvalidAccountKey,
    UnknownError,
    AccountNameExists,
    InvalidCharacterName,
    InvalidProfession,
    PlayerNameExists,
    InvalidAccount,
    InvalidPlayerSex,
    InvalidCharacter,
    InvalidCharactersInString,
    NoMoreCharSlots,
    InvalidGame,
    AllServersFull,
    TokenAuthFailure,
    AccountKeyAlreadyAdded,

    ErrorException = 0xff
};

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

enum class CreatureState : uint8_t
{
#define ENUMERATE_CREATURE_STATE(v) v,
    ENUMERATE_CREATURE_STATES
#undef ENUMERATE_CREATURE_STATE
};

enum class ServerMessageType : uint8_t
{
    Unknown = 0,
    Info,
    Roll, // /roll result
    Age,  // /age
    Hp,   // /hp
    Xp,   // /xp
    Deaths,
    Pos,  // /pos
    PlayerNotOnline,
    PlayerGotMessage,
    NewMail,
    MailSent,
    MailNotSent,
    MailboxFull,
    MailDeleted,
    ServerId,
    PlayerResigned,
    PlayerQueued,
    PlayerUnqueued,

    Instances,
    GMInfo,
    AdminMessage,

    PlayerNotFound,

    UnknownCommand = 0xff
};

enum class ChatChannel : uint8_t
{
    Unknown = 0,
    General,
    Guild,
    Party,
    Allies,
    Trade,
    Whisper,
};

/// Packets sent by the server
enum class ServerPacketType : uint8_t
{
    __First = 0,
    NoError = __First,
    // Let's start with 1
#define ENUMERATE_SERVER_PACKET_CODE(v) v,
    ENUMERATE_SERVER_PACKET_CODES
#undef ENUMERATE_SERVER_PACKET_CODE
    __Last = 0xFF
};

inline const char* GetServerPacketTypeName(ServerPacketType type)
{
    switch (type)
    {
    case ServerPacketType::__First: return "__First";
#define ENUMERATE_SERVER_PACKET_CODE(v) case ServerPacketType::v: return #v;
        ENUMERATE_SERVER_PACKET_CODES
#undef ENUMERATE_SERVER_PACKET_CODE
    case ServerPacketType::__Last: return "__Last";
    default:
        return "Unknown";
    }
}

enum class ObjectCallType : uint8_t
{
    None = 0,
    Follow,
    Attack,
    UseSkill,
    Using,
    TalkingTo,
    PickingUp,
    Target,
};

enum class ObjectProgressType : uint8_t
{
    XPIncreased,
    GotSkillPoint,
    LevelAdvance,
    AttribPointsGain,
};

enum class ResourceType : uint8_t
{
    Health = 1,
    Energy,
    Adrenaline,
    Overcast,
    HealthRegen,
    EnergyRegen,
    MaxHealth,
    MaxEnergy,
    Morale,
};

enum class SkillError : uint8_t
{
#define ENUMERATE_SKILL_ERROR_CODE(v) v,
    ENUMERATE_SKILL_ERROR_CODES
#undef ENUMERATE_SKILL_ERROR_CODE
};

enum class AttackError : uint8_t
{
    None = 0,
    InvalidTarget,
    TargetUndestroyable,
    Interrupted,
    NoTarget,
    TargetObstructed,
    TargetDodge,
    TargetMissed,
};

enum class PlayerErrorValue : uint8_t
{
    None = 0,
    InventoryFull,
    ChestFull,
    NotAllowedWhileTrading,
    TradingPartnerInvalid,
    TradingPartnerQueueing,
    TradingPartnerTrading,
    AlreadyTradingWithThisTarget,
    NotEnoughMoney,
    NoEnoughMaterials,
    ItemNotAvailable,
    DropForOtherPlayer,
};

enum class GameObjectType : uint8_t
{
    Unknown = 0,
    Static,       // GameObject
    TerrainPatch, // GameObject
    // -- Bellow all objects are sent to player when they spawn ----------------
    __SentToPlayer, // Not an actual object type, all bellow is sent to the player
    ItemDrop,     // GameObject
    AreaOfEffect, // GameObject: Area that affects actors in it, e.g. a well
    // Bellow are all Actors
    __Actors,
    Projectile = __Actors, // Actor
    Npc,
    Player, // Human player
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
    ObjectSpawnFieldGroupMask = 1 << 9,
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
    ObjectSpawnDataFieldPvpCharacter = 1 << 6,
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
    EnterInstance,    // /enterinstance <uuid>
    Instances,        // /instances
    Die,              // /die (only GM)
    GodMode,          // /gm  Toggle god mode
    GotoPlayer,       // /gotoplayer <name>
    GMInfo,           // /gminfo <message>  GM info massage

    // Internal
    Internal = 100,
    Help = Internal, // /help
    Clear,
    History,         // /history show chat input history
    Ip,              // /ip
    ServerId,        // /id
    PrefPath,        // /prefpath
    Quit,
    Time,
    ClientPrediction,
};

}

}
