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
    frameTime_ = AbTick();
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
    frameTime_ = AbTick();
    isOpen_ = true;
}

void OutputMessagePool::Send(std::shared_ptr<OutputMessage> message)
{
    lock_.lock();
    OutputMessage::State state = message->GetState();
    lock_.unlock();

    if (state == OutputMessage::State::StateAllocatedNoAutosend)
    {
#ifdef DEBUG_NET
        LOG_DEBUG << "Sending message" << std::endl;
#endif
        if (auto conn = message->GetConnection())
        {
            if (!conn->Send(message))
            {
                // Send only fails when connection is closing (or in error state)
                // This call will free the message
                message->GetProtocol()->OnSendMessage(message);
            }
        }
#ifdef DEBUG_NET
        else
            LOG_ERROR << "connection == null" << std::endl;
#endif
    }
#ifdef DEBUG_NET
    else
        LOG_WARNING << "state != OutputMessage::State::StateAllocatedNoAutosend" << std::endl;
#endif
}

void OutputMessagePool::SendAll()
{
    std::lock_guard<std::recursive_mutex> lockClass(lock_);

    for (auto it = toAddQueue_.begin(); it != toAddQueue_.end();)
    {
        // Drop messages older than 10 seconds
        if (AbTick() - (*it)->GetFrame() > 10 * 1000)
        {
            (*it)->GetProtocol()->OnSendMessage(*it);
            it = toAddQueue_.erase(it);
            continue;
        }

        (*it)->SetState(OutputMessage::State::StateAllocated);
        autosendOutputMessages_.push_back(*it);
    }

    toAddQueue_.clear();

    for (auto it = autosendOutputMessages_.begin(); it != autosendOutputMessages_.end();)
    {
        std::shared_ptr<OutputMessage> omsg = *it;
        bool v = omsg->GetMessageLength() > 1024 || (frameTime_ - omsg->GetFrame() > 10);
        if (v)
        {
            // It will send only messages bigger then 1 kb or with a lifetime greater than 10 ms
#ifdef DEBUG_NET
            LOG_DEBUG << "Sending message" << std::endl;
#endif
            if (omsg->GetConnection())
            {
                if (!omsg->GetConnection()->Send(omsg))
                {
                    omsg->GetProtocol()->OnSendMessage(omsg);
                }
            }
#ifdef DEBUG_NET
            else
                LOG_ERROR << "connection = NULL" << std::endl;
#endif
            it = autosendOutputMessages_.erase(it);
        }
        else
            ++it;
    }

}

std::shared_ptr<OutputMessage> OutputMessagePool::GetOutputMessage(Protocol* protocol,
    bool autosend /* = true */)
{
    if (!isOpen_)
        return std::shared_ptr<OutputMessage>();

    std::lock_guard<std::recursive_mutex> lockClass(lock_);

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

void OutputMessagePool::AddToAutoSend(std::shared_ptr<OutputMessage> msg)
{
    lock_.lock();
    toAddQueue_.push_back(msg);
    lock_.unlock();
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

    message->FreeMessage();

    lock_.lock();
    outputMessages_.push_back(message);
    lock_.unlock();
}

void OutputMessagePool::ConfigureOutputMessage(std::shared_ptr<OutputMessage> message,
    Protocol* protocol, bool autosend)
{
    message->Reset();
    if (autosend)
    {
        message->SetState(OutputMessage::State::StateAllocated);
        autosendOutputMessages_.push_back(message);
    }
    else
    {
        message->SetState(OutputMessage::State::StateAllocatedNoAutosend);
    }

    std::shared_ptr<Connection> connection = protocol->GetConnection();
    assert(connection);

    message->SetProtocol(protocol);
    protocol->AddRef();
    message->SetConnection(connection);
    connection->AddRef();
    message->SetFrame(frameTime_);
}
