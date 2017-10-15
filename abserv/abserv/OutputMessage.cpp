#include "stdafx.h"
#include "OutputMessage.h"
#include "System.h"
#include "Dispatcher.h"
#include "Logger.h"

OutputMessagePool::OutputMessagePool()
{
    for (unsigned i = 0; i < OUTPUT_POOL_SIZE; ++i)
    {
        OutputMessage* msg = new OutputMessage();
        outputMessages_.push_back(msg);
    }
    frameTime_ = SysTime();
}

OutputMessagePool::~OutputMessagePool()
{
    for (auto it = outputMessages_.begin(); it != outputMessages_.end(); ++it)
    {
        delete *it;
    }
    outputMessages_.clear();
}

void OutputMessagePool::StartExecutionFrame()
{
    frameTime_ = SysTime();
    isOpen_ = true;
}

void OutputMessagePool::Send(std::shared_ptr<OutputMessage> message)
{
    outputPoolLock_.lock();
    OutputMessage::State state = message->GetState();
    outputPoolLock_.unlock();

    if (state == OutputMessage::State::STATE_ALLOCATED_NO_AUTOSEND)
    {
        if (auto conn = message->GetConnection())
        {
            if (!conn->Send(message))
            {
                // Send only fails when connection is closing (or in error state)
                // This call will free the message
                message->GetProtocol()->OnSendMessage(message);
            }
        }
        else
            LOG_ERROR << "connection == null" << std::endl;
    }
    else
        LOG_WARNING << "state != OutputMessage::State::STATE_ALLOCATED_NO_AUTOSEND" << std::endl;
}

std::shared_ptr<OutputMessage> OutputMessagePool::GetOutputMessage(Protocol* protocol,
    bool autosend /* = true */)
{
    if (!isOpen_)
        return std::shared_ptr<OutputMessage>();

    std::lock_guard<std::recursive_mutex> lockClass(outputPoolLock_);

    if (protocol->GetConnection() == nullptr)
        return std::shared_ptr<OutputMessage>();

    if (outputMessages_.empty())
    {
        OutputMessage* msg = new OutputMessage();
        outputMessages_.push_back(msg);
    }

    std::shared_ptr<OutputMessage> outputMessage;
    outputMessage.reset(outputMessages_.back(),
        std::bind(&OutputMessagePool::ReleaseMessage, this, std::placeholders::_1));

    outputMessages_.pop_back();

    ConfigureOutputMessage(outputMessage, protocol, autosend);
    return outputMessage;
}

void OutputMessagePool::ReleaseMessage(OutputMessage* message)
{
    Dispatcher::Instance.Add(
        CreateTask(std::bind(&OutputMessagePool::InternalReleaseMessage, this, message)),
        true
    );
}

void OutputMessagePool::InternalReleaseMessage(OutputMessage* message)
{
    if (Protocol* p = message->GetProtocol())
    {
        p->ReleaseRef();
    }

    if (message->GetConnection())
    {
        message->GetConnection()->ReleaseRef();
    }

    message->Free();

    outputPoolLock_.lock();
    outputMessages_.push_back(message);
    outputPoolLock_.unlock();
}

void OutputMessagePool::ConfigureOutputMessage(std::shared_ptr<OutputMessage> message,
    Protocol* protocol, bool autosend)
{
    message->Reset();
    if (autosend)
    {
        message->SetState(OutputMessage::State::STATE_ALLOCATED);
        autosendOutputMessages_.push_back(message);
    }
    else
    {
        message->SetState(OutputMessage::State::STATE_ALLOCATED_NO_AUTOSEND);
    }

    std::shared_ptr<Connection> connection = protocol->GetConnection();
    assert(connection);

    message->SetProtocol(protocol);
    protocol->AddRef();
    message->SetConnection(connection);
    connection->AddRef();
    message->SetFrame(frameTime_);
}
