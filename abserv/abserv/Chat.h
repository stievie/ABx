#pragma once

namespace Game {

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
public:
    ChatChannel() = default;
    virtual ~ChatChannel() = default;
};

class Chat
{
private:
    // Type | ID
    std::map<uint64_t, std::unique_ptr<ChatChannel>> channels_;
public:
    Chat() = default;
    ~Chat() = default;
    // non-copyable
    Chat(const Chat&) = delete;
    Chat& operator=(const Chat&) = delete;

    ChatChannel* Get(uint8_t type, uint32_t id);

    static Chat Instance;
};

}
