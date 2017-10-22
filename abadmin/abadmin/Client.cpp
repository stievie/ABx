#include "stdafx.h"
#include "Client.h"
#include <iostream>
#include "Rsa.h"

Client::Client() :
    connected_(false)
{
}


Client::~Client()
{
}

void Client::SetSocketMode(bool blocking)
{
#if defined WIN32 || defined __WINDOWS__
    // Set the socket I/O mode; iMode = 0 for blocking; iMode != 0 for non-blocking
    unsigned long mode;
    if (blocking)
        mode = 0;
    else
        mode = 1;
    ioctlsocket(socket_, FIONBIO, &mode);
#else
    int flags = fcntl(g_socket, F_GETFL);
    if (blocking)
        flags &= (~O_NONBLOCK);
    else
        flags |= O_NONBLOCK;
    fcntl(g_socket, F_SETFL, flags);
#endif
}

SocketCode Client::SendMsg(NetworkMessage& msg, uint32_t* key)
{
    SocketCode ret = msg.WriteToSocket(socket_);
    msg.Reset();

    if (ret == SocketCodeOK)
    {
        if (key)
        {
            msg.SetEncryptionState(true);
            msg.SetEncryptionKey(key);
        }

        do
        {
            ret = msg.ReadFromSocket(socket_);
            if (ret == SocketCodeOK)
            {
                char retCode = msg.PeekByte();
                if (retCode == AP_MSG_ERROR)
                {
                    msg.GetByte();
                    std::string errorDescr = msg.GetString();
                    std::cout << "MSG_ERROR " << errorDescr << std::endl;
                    return SocketCodeError;
                }
            }
            else if (ret == SocketCodeError)
            {
                std::cout << "Error while reading" << std::endl;
                return ret;
            }
            else
            {
                AB_SLEEP(5000);
                NetworkMessage msg_keepAlive;
                msg_keepAlive.AddByte(AP_MSG_KEEP_ALIVE);
                if (msg_keepAlive.WriteToSocket(socket_) != SocketCodeOK)
                {
                    return SocketCodeError;
                }
            }
        } while (ret == SocketCodeTimeout);
    }
    else if (ret == SocketCodeError)
        std::cout << "Error sending message" << std::endl;
    return ret;
}

bool Client::Connect(const std::string& pass)
{
    if (connected_)
        return false;

    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    uint32_t remoteIp = inet_addr(host_.c_str());

    if (remoteIp == INADDR_NONE)
    {
        struct hostent* hp = gethostbyname(host_.c_str());
        if (hp != 0)
            remoteIp = *(long*)hp->h_addr;
        else
        {
            closesocket(socket_);
            std::cout << "Can not resolve server" << std::endl;
            return false;
        }
    }

    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = remoteIp;
    serveraddr.sin_port = htons(port_);

    if (connect(socket_, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        closesocket(socket_);
        std::cout << "Can not connect to server" << std::endl;
        return false;
    }
    std::cout << "Connected to " << host_ << std::endl;

    SetSocketMode(false);

    NetworkMessage msg;
    msg.AddByte(0xFE);      // Protocol ID
    if (msg.WriteToSocket(socket_) != SocketCodeOK)
    {
        closesocket(socket_);
        std::cout << "Error while sending first byte" << std::endl;
        return false;
    }
    msg.Reset();

    if (msg.ReadFromSocket(socket_) != SocketCodeOK)
    {
        closesocket(socket_);
        std::cout << "Error while reading hello" << std::endl;
        return false;
    }

    char byte = msg.GetByte();
    if (byte != AP_MSG_HELLO)
    {
        closesocket(socket_);
        std::cout << "No valid server hello" << std::endl;
        return false;
    }

    msg.Get<uint32_t>();
    std::string strversion = msg.GetString();
    uint16_t security = msg.Get<uint16_t>();
    uint32_t options = msg.Get<uint32_t>();
    if (security & REQUIRE_ENCRYPTION)
    {
        strversion += " encryption";
        if (options & ENCRYPTION_RSA1024XTEA)
            strversion += "(RSA1024XTEA)";
        else
            strversion += "(Not supported)";
    }
    if (security & REQUIRE_LOGIN)
        strversion += " login";

    std::cout << "Hello from " << strversion << std::endl;

    // Set encryption
    if (security & REQUIRE_ENCRYPTION)
    {
        if (options & ENCRYPTION_RSA1024XTEA)
        {
            // Get public key
            msg.Reset();
            msg.AddByte(AP_MSG_KEY_EXCHANGE);
            msg.AddByte(ENCRYPTION_RSA1024XTEA);

            if (SendMsg(msg) != SocketCodeOK)
            {
                closesocket(socket_);
                std::cout << "Error getting public key" << std::endl;
                return false;
            }

            char retCode = msg.GetByte();
            if (retCode == AP_MSG_KEY_EXCHANGE_OK)
                std::cout << "Key exchange OK" << std::endl;
            else if (retCode == AP_MSG_KEY_EXCHANGE_FAILED)
            {
                std::string errorDescr = msg.GetString();
                closesocket(socket_);
                std::cout << "Can not get public key " << errorDescr << std::endl;
                return false;
            }
            else
            {
                closesocket(socket_);
                std::cout << "Unknown server response" << std::endl;
                return false;
            }

            unsigned char keyType = msg.GetByte();
            if (keyType != ENCRYPTION_RSA1024XTEA)
            {
                closesocket(socket_);
                std::cout << "No valid key" << std::endl;
                return false;
            }

            uint32_t rsaMod[32];
            for (unsigned i = 0; i < 32; ++i)
            {
                rsaMod[i] = msg.Get<uint32_t>();
            }
            Rsa::Instance.SetPublicKey((char*)rsaMod, "65537");

            uint32_t randomKey[32];
            for (unsigned i = 0; i < 32; ++i)
            {
                randomKey[i] = rand() << 16 ^ rand();
            }

            msg.Reset();

            msg.AddByte(AP_MSG_ENCRYPTION);
            msg.AddByte(ENCRYPTION_RSA1024XTEA);
            msg.AddByte(0);
            for (unsigned i = 0; i < 31; ++i)
            {
                msg.Add<uint32_t>(randomKey[i]);
            }
            msg.AddByte(0);
            msg.AddByte(0);
            msg.AddByte(0);

            msg.RSAEncrypt();

            if (SendMsg(msg, randomKey) != SocketCodeOK)
            {
                closesocket(socket_);
                std::cout << "Error while sending key" << std::endl;
                return false;
            }

            retCode = msg.GetByte();
            if (retCode == AP_MSG_ENCRYPTION_OK)
                std::cout << "Encryption OK" << std::endl;
            else if (retCode == AP_MSG_ENCRYPTION_FAILED)
            {
                std::string errDescr = msg.GetString();
                closesocket(socket_);
                std::cout << "Can not set private keys " << errDescr << std::endl;
                return false;
            }
            else
            {
                closesocket(socket_);
                std::cout << "Unknown response" << std::endl;
                return false;
            }
        }
        else
        {
            closesocket(socket_);
            std::cout << "Can not initiate encryption" << std::endl;
            return false;
        }
    }

    if (security & REQUIRE_LOGIN)
    {
        msg.Reset();
        msg.AddByte(AP_MSG_LOGIN);
        msg.AddString(pass);

        if (SendMsg(msg) != SocketCodeOK)
        {
            closesocket(socket_);
            std::cout << "Error sending login" << std::endl;
            return false;
        }

        char retCode = msg.GetByte();
        if (retCode == AP_MSG_LOGIN_OK)
            std::cout << "Login OK" << std::endl;
        else if (retCode == AP_MSG_LOGIN_FAILED)
        {
            std::string errDescr = msg.GetString();
            closesocket(socket_);
            std::cout << "Login failed " << errDescr << std::endl;
            return false;
        }
        else
        {
            closesocket(socket_);
            std::cout << "Unknown response" << std::endl;
            return false;
        }
    }

    connected_ = true;
    return true;
}

bool Client::Disconnect()
{
    if (!connected_)
        return false;

    closesocket(socket_);
    socket_ = SOCKET_ERROR;
    std::cout << "Disconnected" << std::endl;
    connected_ = false;

    return true;
}

bool Client::SendCommand(char cmdByte, char* command)
{
    NetworkMessage msg;
    msg.AddByte(AP_MSG_COMMAND);
    msg.AddByte(cmdByte);
    if (command)
        msg.AddString(command);

    SocketCode ret = SendMsg(msg);
    if (ret == SocketCodeTimeout)
        return false;

    char retCode = msg.GetByte();
    if (retCode == AP_MSG_COMMAND_OK)
        return true;
    if (retCode == AP_MSG_COMMAND_FAILED)
    {
        std::string errorDesc = msg.GetString();
        std::cout << "Error: " << errorDesc << std::endl;
        return false;
    }
    std::cout << "Unknown return code" << std::endl;
    return false;
}
