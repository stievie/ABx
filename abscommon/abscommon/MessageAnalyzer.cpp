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

#include "MessageAnalyzer.h"
#include "MessageDecoder.h"
#include "Logger.h"

namespace Net {

void MessageAnalyzer::Analyze(const NetworkMessage& message)
{
    using namespace AB::Packets::Server;
    MessageDecoder decoder(message);
    for (;;)
    {
        auto code = decoder.GetNext();
        if (!code.has_value())
            return;

        switch (code.value())
        {
#define ENUMERATE_SERVER_PACKET_CODE(v) case AB::GameProtocol::ServerPacketType::v:                 \
            {                                                                                       \
                v packet = AB::Packets::Get<v>(decoder);                                            \
                if (onPacket_)                                                                      \
                    onPacket_(code.value(), &packet);                                               \
                break;                                                                              \
            }
            ENUMERATE_SERVER_PACKET_CODES
#undef ENUMERATE_SERVER_PACKET_CODE
        default:
            LOG_ERROR << "Unknown packet type " << static_cast<int>(code.value()) << std::endl;
            break;
        }
    }
}

}
