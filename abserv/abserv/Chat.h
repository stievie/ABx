#pragma once

#include "StringHash.h"
#include <AB/ProtocolCodes.h>

namespace Game {

class Game;
class Player;
class Party;
class Npc;

enum class ChatType : uint8_t
{
    /// Local map chat
    Map = AB::GameProtocol::ChatChannelGeneral,       // ID = GameID
    /// Guild messages get all guild members on all servers
    Guild = AB::GameProtocol::ChatChannelGuild,     // ID = StringHash(Guild.uuid)
    Party = AB::GameProtocol::ChatChannelParty,     // ID = PartyID
    /// There may be allies on the map that do not belong to the party
    Allies = AB::GameProtocol::ChatChannelAllies,    //
    /// Trade messages get all players on all servers
    Trade = AB::GameProtocol::ChatChannelTrade,     // ID = 0
    Whisper = AB::GameProtocol::ChatChannelWhisper,   // ID = PlayerID
};

class ChatChannel
{
protected:
    uint64_t id_;
public:
    ChatChannel(uint64_t id) :
        id_(id)
    {}
    virtual ~ChatChannel() = default;
    virtual bool Talk(Player* player, const std::string& text) {
        AB_UNUSED(player);
        AB_UNUSED(text);
        return false;
    }
    virtual bool TalkNpc(Npc* npc, const std::string& text)
    {
        AB_UNUSED(npc);
        AB_UNUSED(text);
        return false;
    }
};

class GameChatChannel : public ChatChannel
{
private:
    std::weak_ptr<Game> game_;
public:
    GameChatChannel(uint64_t id);
    bool Talk(Player* player, const std::string& text) override;
    bool TalkNpc(Npc* npc, const std::string& text) override;
};

class PartyChatChannel : public ChatChannel
{
public:
    PartyChatChannel(uint64_t id) :
        ChatChannel(id),
        party_(nullptr)
    { }
    bool Talk(Player* player, const std::string& text) override;
    bool TalkNpc(Npc* npc, const std::string& text) override;
    Party* party_;
};

class WhisperChatChannel : public ChatChannel
{
private:
    /// The recipient
    std::weak_ptr<Player> player_;
    std::string playerUuid_;
public:
    WhisperChatChannel(uint64_t id);
    WhisperChatChannel(const std::string& playerUuid);
    bool Talk(Player* player, const std::string& text) override;
    bool Talk(const std::string& playerName, const std::string& text);
};

/// Trade channel, all players on all servers will get it
class TradeChatChannel : public ChatChannel
{
public:
    TradeChatChannel() :
        ChatChannel(0)
    { }
    bool Talk(Player* player, const std::string& text) override;
    void Broadcast(const std::string& playerName, const std::string& text);
};

class GuildChatChannel : public ChatChannel
{
private:
    std::string guildUuid_;
public:
    GuildChatChannel(const std::string& guildUuid) :
        ChatChannel(Utils::StringHashRt(guildUuid.c_str())),
        guildUuid_(guildUuid)
    { }
    bool Talk(Player* player, const std::string& text) override;
    void Broadcast(const std::string& playerName, const std::string& text);
};

class Chat
{
private:
    // Type | ID
    std::map<std::pair<ChatType, uint64_t>, std::shared_ptr<ChatChannel>> channels_;
    std::shared_ptr<ChatChannel> tradeChat_;
public:
    Chat();
    ~Chat() = default;
    // non-copyable
    Chat(const Chat&) = delete;
    Chat& operator=(const Chat&) = delete;

    std::shared_ptr<ChatChannel> Get(ChatType type, uint64_t id);
    std::shared_ptr<ChatChannel> Get(ChatType type, const std::string& uuid);
    void Remove(ChatType type, uint64_t id);
    void CleanChats();

    static Chat Instance;
};

}
