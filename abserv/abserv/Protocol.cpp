#include "stdafx.h"
#include "Protocol.h"
#include "NetworkMessage.h"
#include "OutputMessage.h"

namespace Net {

void Protocol::XTEAEncrypt(OutputMessage& message)
{
    int32_t msgLength = message.GetMessageLength();

    uint32_t n;
    // Add bytes until we reach a multiple of 8
    if ((msgLength % 8) != 0)
    {
        n = 8 - (msgLength % 8);
        message.AddPaddingBytes(n);
        msgLength += n;
    }
}

void Protocol::XTEADecrypt(NetworkMessage& message)
{
}

void Protocol::OnSendMessage(std::shared_ptr<OutputMessage> message)
{
}

void Protocol::OnRecvMessage(NetworkMessage& message)
{
}

void Protocol::Release()
{
}

}
