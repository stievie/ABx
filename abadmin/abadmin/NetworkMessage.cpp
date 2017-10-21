#include "stdafx.h"
#include "NetworkMessage.h"


NetworkMessage::NetworkMessage()
{
    Reset();
}

SocketCode NetworkMessage::ReadFromSocket(SOCKET socket, int timeout)
{
    int readBytes = 0;
    int sleepTime = 0;
    size_ = 0;

    do
    {
        // just read the size to avoid reading 2 messages at once
        int b = recv(socket, (char*)buffer_ + readBytes, 2 - readBytes, 0);
        if (b <= 0)
        {
            int errNum = ::WSAGetLastError();
            if (errNum == WSAEWOULDBLOCK)
            {
                b = 0;
                AB_SLEEP(10);
                sleepTime += 10;
                if (sleepTime > timeout)
                {
                    Reset();
                    return SocketCodeError;
                }
            }
            else
            {
                Reset();
                return SocketCodeError;
            }
        }
        readBytes += b;
    } while (readBytes < 2);

    size_ = readBytes;

    return SocketCodeOK;
}

SocketCode NetworkMessage::WriteToSocket(SOCKET socket, int timeout)
{
    return SocketCodeOK;
}

void NetworkMessage::Reset()
{
    size_ = 0;
    readPos_ = 0;
}
