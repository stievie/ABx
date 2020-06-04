/**
 * Copyright 2020 Stefan Ascher
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

#include "MessageFilter.h"
#include "MessageDecoder.h"
#include <sa/Assert.h>
#include <abscommon/Logger.h>
#include "Game.h"
#include "Player.h"

namespace Net {

void MessageFilter::Execute(const Game::Game& game, const Game::Player& player, const NetworkMessage& source, NetworkMessage& dest)
{
    MessageDecoder decoder(source);
    for (;;)
    {
        auto code = decoder.GetNext();
        if (!code.has_value())
            return;

        switch (code.value())
        {
#define ENUMERATE_SERVER_PACKET_CODE(v) case AB::GameProtocol::ServerPacketType::v:                 \
            {                                                                                       \
                AB::Packets::Server::v packet = AB::Packets::Get<AB::Packets::Server::v>(decoder);  \
                constexpr size_t id = sa::StringHash(sa::TypeName<AB::Packets::Server::v>::Get());  \
                if (events_.HasSubscribers<bool(const Game::Game&, const Game::Player&, AB::Packets::Server::v&)>(id))                      \
                {                                                                                   \
                    if (!events_.CallOne<bool(const Game::Game&, const Game::Player&, AB::Packets::Server::v&)>(id, game, player, packet))  \
                        continue;                                                                   \
                }                                                                                   \
                PassThrough(dest, AB::GameProtocol::ServerPacketType::v, packet);                   \
                break;                                                                              \
            }
            ENUMERATE_SERVER_PACKET_CODES
#undef ENUMERATE_SERVER_PACKET_CODE
        default:
            dest.Add(code.value());
            break;
        }
    }
}

}
