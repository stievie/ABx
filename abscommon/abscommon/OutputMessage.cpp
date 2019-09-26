#include "stdafx.h"
#include "OutputMessage.h"
#include "Utils.h"
#include "Dispatcher.h"
#include "Scheduler.h"
#include "Subsystems.h"
#include "Protocol.h"

namespace Net {

const std::chrono::milliseconds OUTPUTMESSAGE_AUTOSEND_DELAY{ 10 };
OutputMessagePool::MessagePool OutputMessagePool::sOutputMessagePool;

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

sa::SharedPtr<OutputMessage> OutputMessagePool::GetOutputMessage()
{
    auto* ptr = sOutputMessagePool.allocate(1, nullptr);
    assert(ptr);
    ptr->Reset();
    return sa::SharedPtr<OutputMessage>(ptr);
}

void OutputMessagePool::DeleteOutputMessage(OutputMessage* p)
{
    sOutputMessagePool.deallocate(p, 1);
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
