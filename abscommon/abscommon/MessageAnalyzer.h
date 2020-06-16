#pragma once

#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include "NetworkMessage.h"
#include <sa/StringHash.h>
#include <sa/TypeName.h>
#include <functional>

namespace Net {

class MessageAnalyzer
{
public:
    void Analyze(const NetworkMessage& message);

    std::function<void(AB::GameProtocol::ServerPacketType type)> onPacket_;
};

}
