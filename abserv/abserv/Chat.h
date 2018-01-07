#pragma once

namespace Game {

class Game;
class Player;

enum ChatType : uint8_t
{
    // Alliance chat
    ChannelAlliance = 0x00,  // ID = AllianceID
    ChannelGuild = 0x01,     // ID = GuildID
    // Local map chat
    ChannelMap = 0x02,       // ID = GameID
    ChannelTrade = 0x03,     // ID = GameID
    // There may be allies on the map that do not belong to the party
    ChannelAllies = 0x04,    //
    ChannelParty = 0x05,     // ID = PartyID
    ChannelWhisper = 0x06,   // ID = PlayerID
};

class ChatChannel
{
protected:
    uint32_t id_;
public:
    ChatChannel(uint32_t id) :
        id_(id)
    {}
    virtual ~ChatChannel() = default;
    virtual bool Talk(Player* player, const std::string& text) {
        AB_UNUSED(player);
        AB_UNUSED(text);
        return false;
    }
};

class GameChatChannel : public ChatChannel
{
private:
    std::weak_ptr<Game> game_;
public:
    GameChatChannel(uint32_t id);
    bool Talk(Player* player, const std::string& text) override;
};

class WhisperChatChannel : public ChatChannel
{
private:
    std::weak_ptr<Player> player_;
public:
    WhisperChatChannel(uint32_t id);
    bool Talk(Player* player, const std::string& text) override;
};

class Chat
{
private:
    // Type | ID
    std::map<uint64_t, std::shared_ptr<ChatChannel>> channels_;
public:
    Chat() = default;
    ~Chat() = default;
    // non-copyable
    Chat(const Chat&) = delete;
    Chat& operator=(const Chat&) = delete;

    std::shared_ptr<ChatChannel> Get(uint8_t type, uint32_t id);
    void Remove(uint8_t type, uint32_t id);

    static Chat Instance;
};

}
