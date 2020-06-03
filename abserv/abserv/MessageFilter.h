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

#pragma once

#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <abscommon/NetworkMessage.h>
#include <sa/Events.h>
#include <sa/TypeName.h>

namespace Net {

// Oh, well, I'm not sure about this...
// It looks a bit like read-only code.

class MessageFilter
{
private:
    using FilterEvents = sa::Events<
#define ENUMERATE_SERVER_PACKET_CODE(v) bool(AB::Packets::Server::v&),
        ENUMERATE_SERVER_PACKET_CODES
#undef ENUMERATE_SERVER_PACKET_CODE
        bool(void)
    >;
    const NetworkMessage& source_;
    NetworkMessage& dest_;
    FilterEvents events_;
    template<typename T>
    void PassThrough(AB::GameProtocol::ServerPacketType type, T& packet)
    {
        dest_.AddByte(type);
        AB::Packets::Add(packet, dest_);
    }
public:
    MessageFilter(const NetworkMessage& source, NetworkMessage& dest) :
        source_(source),
        dest_(dest)
    { }

    template<typename T, typename Callback>
    size_t Subscribe(Callback&& callback)
    {
        constexpr size_t id = sa::StringHash(sa::TypeName<T>::Get());
        return events_.Subscribe<bool(T&)>(id, std::move(callback));
    }
    void Execute();
};

}
