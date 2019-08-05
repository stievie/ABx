#include "stdafx.h"
#include "MatchQueues.h"

void MatchQueues::Add(const std::string& mapUuid, const std::string& playerUuid)
{
    Queue* queue = GetQueue(mapUuid);
    if (!queue)
    {
        LOG_ERROR << "Unable to get queue for game " << mapUuid << std::endl;
        return;
    }
    queue->Add(playerUuid);
    parties_.emplace(playerUuid, mapUuid);
}

void MatchQueues::Remove(const std::string& playerUuid)
{
    auto it = parties_.find(playerUuid);
    if (it == parties_.end())
        return;
    Queue* queue = GetQueue((*it).second);
    if (!queue)
        return;
    queue->Remove(playerUuid);
    parties_.erase(it);
}

Queue* MatchQueues::GetQueue(const std::string& mapUuid)
{
    auto it = queues_.find(mapUuid);
    if (it == queues_.end())
    {
        auto queue = std::make_unique<Queue>(mapUuid);
        if (queue->Load())
        {
            queues_.emplace(mapUuid, std::move(queue));
            it = queues_.find(mapUuid);
        }
    }
    if (it == queues_.end())
        return nullptr;
    return (*it).second.get();
}

void MatchQueues::Update(uint32_t timeElapsed)
{
    for (const auto& q : queues_)
    {
        q.second->Update(timeElapsed);
    }
}
