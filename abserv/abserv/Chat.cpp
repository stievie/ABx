#include "stdafx.h"
#include "Chat.h"

namespace Game {

Chat Chat::Instance;

ChatChannel* Chat::Get(uint8_t type, uint32_t id)
{
    uint64_t channelId = ((uint64_t)type << 32) | id;
    auto it = channels_.find(channelId);
    if (it != channels_.end())
        return (*it).second.get();

    std::unique_ptr<ChatChannel> c = std::make_unique<ChatChannel>();
    ChatChannel* result = c.get();
    channels_.emplace(channelId, std::move(c));
    return result;
}

}
