#include "stdafx.h"
#include "OutputMessage.h"
#include "System.h"

OutputMessagePool::OutputMessagePool()
{
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

            }
        }
    }
}
