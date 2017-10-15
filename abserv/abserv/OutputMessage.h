#pragma once

#include <memory>
#include <stdint.h>
#include <list>
#include <mutex>
#include "NetworkMessage.h"
#include "Connection.h"
#include "Protocol.h"

#define OUTPUT_POOL_SIZE 100

class OutputMessage : public NetworkMessage
{
public:
    enum State
    {
        STATE_FREE,
        STATE_ALLOCATED,
        STATE_ALLOCATED_NO_AUTOSEND,
        STATE_WAITING
    };
private:
    std::shared_ptr<Connection> connection_;
    Protocol* protocol_;
    OutputMessage::State state_;
    uint32_t outputBufferStart_;
    uint64_t frame_;

    OutputMessage()
    {
        Free();
    }
    void Free()
    {
        SetConnection(std::shared_ptr<Connection>());
        SetProtocol(nullptr);
        frame_ = 0;
        // allocate enough size for headers
        // 2 bytes for unencrypted message size
        // 4 bytes for checksum
        // 2 bytes for encrypted message size
        outputBufferStart_ = 8;
        SetState(OutputMessage::State::STATE_FREE);
    }

    friend class OutputMessagePool;
public:
    OutputMessage(const OutputMessage&) = delete;
    ~OutputMessage() {}

    std::shared_ptr<Connection> GetConnection() { return connection_; }
    void SetConnection(std::shared_ptr<Connection> connection)
    {
        connection_ = connection;
    }
    Protocol* GetProtocol() { return protocol_; }
    void SetProtocol(Protocol* protocol)
    {
        protocol_ = protocol;
    }
    OutputMessage::State GetState() const { return state_; }
    void SetState(OutputMessage::State state)
    {
        state_ = state;
    }
    void SetFrame(uint64_t frame)
    {
        frame_ = frame;
    }
};

class OutputMessagePool
{
private:
    OutputMessagePool();
public:
    ~OutputMessagePool();

    static OutputMessagePool* Instance()
    {
        static OutputMessagePool instance;
        return &instance;
    }

    void StartExecutionFrame();
    void Send(std::shared_ptr<OutputMessage> message);
    std::shared_ptr<OutputMessage> GetOutputMessage(Protocol* protocol, bool autosend = true);
protected:
    typedef std::list<OutputMessage*> InternalOutputMessageList;
    typedef std::list<std::shared_ptr<OutputMessage>> OutputMessageList;
private:
    void ReleaseMessage(OutputMessage* message);
    void InternalReleaseMessage(OutputMessage* message);
    void ConfigureOutputMessage(std::shared_ptr<OutputMessage> message,
        Protocol* protocol, bool autosend);

    InternalOutputMessageList outputMessages_;
    InternalOutputMessageList allOutputMessages_;
    OutputMessageList autosendOutputMessages_;
    OutputMessageList toAddQueue_;
    std::recursive_mutex outputPoolLock_;

    uint64_t frameTime_;
    bool isOpen_;
};