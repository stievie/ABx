#include "stdafx.h"
#include "OutputMessage.h"
#include "Utils.h"
#include "Dispatcher.h"
#include "Scheduler.h"
#include "Subsystems.h"
#include "Protocol.h"
#include "Connection.h"

namespace Net {

const std::chrono::milliseconds OUTPUTMESSAGE_AUTOSEND_DELAY{ 10 };

void OutputMessagePool::SendAll()
{
    // Dispatcher Thread
    for (const auto& proto : bufferedProtocols_)
    {
        auto& msg = proto->GetCurrentBuffer();
        if (msg && msg->GetSize() > 0)
        {
            proto->Send(msg);
            proto->ResetOutputBuffer();
        }
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

#ifdef DEBUG_POOLALLOCATOR
sa::PoolInfo OutputMessagePool::GetPoolInfo()
{
    return gOutputMessagePool.GetInfo();
}
#endif

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

}
