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

#include <string>
#include <memory>
#include <map>
#include <sa/StringHash.h>
#include <AB/ProtocolCodes.h>

namespace Game {

class Game;
class Player;
class Party;
class Npc;

enum class ChatType
{
    /// Returns nullptr
    None = static_cast<int>(AB::GameProtocol::ChatChannel::Unknown),
    /// Local map chat
    Map = static_cast<int>(AB::GameProtocol::ChatChannel::General),     // ID = GameID
    /// Guild messages get all guild members on all servers
    Guild = static_cast<int>(AB::GameProtocol::ChatChannel::Guild),     // ID = StringHash(Guild.uuid)
    Party = static_cast<int>(AB::GameProtocol::ChatChannel::Party),     // ID = PartyID
    /// There may be allies on the map that do not belong to the party
    Allies = static_cast<int>(AB::GameProtocol::ChatChannel::Allies),   //
    /// Trade messages get all players on all servers
    Trade = static_cast<int>(AB::GameProtocol::ChatChannel::Trade),     // ID = 0
    Whisper = static_cast<int>(AB::GameProtocol::ChatChannel::Whisper), // ID = PlayerID
};

class ChatChannel
{
protected:
    uint64_t id_;
public:
    explicit ChatChannel(uint64_t id) :
        id_(id)
    {}
    virtual ~ChatChannel();
    virtual bool Talk(Player&, const std::string&)
    {
        return false;
    }
    virtual bool TalkNpc(Npc&, const std::string&)
    {
        return false;
    }
};

class GameChatChannel : public ChatChannel
{
private:
    std::weak_ptr<Game> game_;
public:
    explicit GameChatChannel(uint64_t id);
    bool Talk(Player& player, const std::string& text) override;
    bool TalkNpc(Npc& npc, const std::string& text) override;
};

class PartyChatChannel : public ChatChannel
{
public:
    explicit PartyChatChannel(uint64_t id) :
        ChatChannel(id),
        party_(nullptr)
    { }
    bool Talk(Player& player, const std::string& text) override;
    bool TalkNpc(Npc& npc, const std::string& text) override;
    Party* party_;
};

class WhisperChatChannel : public ChatChannel
{
private:
    /// The recipient
    std::weak_ptr<Player> player_;
    std::string playerUuid_;
public:
    explicit WhisperChatChannel(uint64_t id);
    WhisperChatChannel(const std::string& playerUuid);
    bool Talk(Player& player, const std::string& text) override;
    bool Talk(const std::string& playerName, const std::string& text);
};

/// Trade channel, all players on all servers will get it
class TradeChatChannel : public ChatChannel
{
public:
    explicit TradeChatChannel() :
        ChatChannel(0)
    { }
    bool Talk(Player& player, const std::string& text) override;
    void Broadcast(const std::string& playerName, const std::string& text);
};

class GuildChatChannel : public ChatChannel
{
private:
    std::string guildUuid_;
public:
    explicit GuildChatChannel(const std::string& guildUuid) :
        ChatChannel(sa::StringHashRt(guildUuid.c_str())),
        guildUuid_(guildUuid)
    { }
    bool Talk(Player& player, const std::string& text) override;
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
};

}
