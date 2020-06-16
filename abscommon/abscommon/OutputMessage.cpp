/**
 * Copyright 2017-2020 Stefan Ascher
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


#include "OutputMessage.h"
#include "Utils.h"
#include "Dispatcher.h"
#include "Scheduler.h"
#include "Subsystems.h"
#include "Protocol.h"
#include "Connection.h"
#include "Subsystems.h"

namespace Net {

const std::chrono::milliseconds OUTPUTMESSAGE_AUTOSEND_DELAY{ 10 };

std::mutex PoolWrapper::lock_;

void OutputMessagePool::SendAll()
{
    // Dispatcher Thread
    for (const auto& proto : bufferedProtocols_)
    {
        auto msg = proto->TakeCurrentBuffer();
        if (msg && msg->GetSize() > 0)
            proto->Send(std::move(msg));
    }

    if (!bufferedProtocols_.empty())
        ScheduleSendAll();
}

void OutputMessagePool::ScheduleSendAll()
{
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(static_cast<uint32_t>(OUTPUTMESSAGE_AUTOSEND_DELAY.count()),
            std::bind(&OutputMessagePool::SendAll, this))
    );
}

sa::PoolInfo OutputMessagePool::GetPoolInfo()
{
    if (PoolWrapper::MessagePool* pool = PoolWrapper::GetOutputMessagePool())
        return pool->GetInfo();
    return { };
}

unsigned OutputMessagePool::GetPoolUsage()
{
    if (PoolWrapper::MessagePool* pool = PoolWrapper::GetOutputMessagePool())
        return pool->GetUsage();
    return 0;
}

sa::SharedPtr<OutputMessage> OutputMessagePool::GetOutputMessage()
{
    return sa::MakeShared<OutputMessage>();
}

void OutputMessagePool::AddToAutoSend(std::shared_ptr<Protocol> protocol)
{
    // Dispatcher Thread
    if (bufferedProtocols_.empty())
        // Create first task
        ScheduleSendAll();

    bufferedProtocols_.emplace_back(protocol);
}

void OutputMessagePool::RemoveFromAutoSend(const std::shared_ptr<Protocol>& protocol)
{
    // Dispatcher Thread
    auto it = std::find(bufferedProtocols_.begin(), bufferedProtocols_.end(), protocol);
    if (it != bufferedProtocols_.end())
    {
        std::swap(*it, bufferedProtocols_.back());
        bufferedProtocols_.pop_back();
    }
}

PoolWrapper::MessagePool* PoolWrapper::GetOutputMessagePool()
{
    auto* pool = GetSubsystem<PoolWrapper::MessagePool>();
    if (!pool)
    {
        LOG_ERROR << "No PoolWrapper::MessagePool" << std::endl;
        return nullptr;
    }
    return pool;
}

OutputMessage::OutputMessage() :
    NetworkMessage()
{ }

}
