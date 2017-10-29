#include "stdafx.h"
#include "OutputMessage.h"
#include "Utils.h"
#include "Dispatcher.h"
#include "Scheduler.h"

#include "DebugNew.h"

namespace Net {

const std::chrono::milliseconds OUTPUTMESSAGE_AUTOSEND_DELAY{ 10 };

void OutputMessagePool::SendAll()
{
    // Dispatcher Thread
    for (auto& proto : bufferedProtocols_)
    {
        auto& msg = proto->GetCurrentBuffer();
        if (msg)
            proto->Send(msg);
    }

    if (!bufferedProtocols_.empty())
        ScheduleSendAll();
}

void OutputMessagePool::ScheduleSendAll()
{
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(static_cast<uint32_t>(OUTPUTMESSAGE_AUTOSEND_DELAY.count()),
            std::bind(&OutputMessagePool::SendAll, this))
    );
}

std::shared_ptr<OutputMessage> OutputMessagePool::GetOutputMessage()
{
    return std::make_shared<OutputMessage>();
}

void OutputMessagePool::AddToAutoSend(std::shared_ptr<Protocol> protocol)
{
    // Dispatcher Thread
    if (bufferedProtocols_.empty())
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
