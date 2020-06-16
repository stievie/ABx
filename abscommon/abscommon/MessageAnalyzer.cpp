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
