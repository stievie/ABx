#include "stdafx.h"
#include "MatchQueues.h"

void MatchQueues::Add(const std::string& mapUuid, const std::string& playerUuid)
{
    std::lock_guard<std::mutex> lock(lock_);
    Queue* queue = GetQueue(mapUuid);
    if (!queue)
    {
        LOG_ERROR << "Unable to get queue for game " << mapUuid << std::endl;
        return;
    }
    queue->Add(playerUuid);
    players_.emplace(playerUuid, mapUuid);
}

void MatchQueues::Remove(const std::string& playerUuid)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = players_.find(playerUuid);
    if (it == players_.end())
    {
        LOG_WARNING << "Player not found " << playerUuid << std::endl;
        return;
    }
    Queue* queue = GetQueue((*it).second);
    if (!queue)
    {
        LOG_WARNING << "No Queue for map " << (*it).second << " found" << std::endl;
        return;
    }
    queue->Remove(playerUuid);
    players_.erase(it);
}

Queue* MatchQueues::GetQueue(const std::string& mapUuid)
{
    auto it = queues_.find(mapUuid);
    if (it == queues_.end())
    {
        auto queue = std::make_unique<Queue>(mapUuid);
        if (queue->Load())
        {
            auto res = queues_.emplace(mapUuid, std::move(queue));
            if (res.second)
                it = res.first;
        }
    }
    if (it == queues_.end())
        return nullptr;
    return (*it).second.get();
}

void MatchQueues::Update(uint32_t timeElapsed)
{
    std::lock_guard<std::mutex> lock(lock_);
    // Delete empty queues
    auto i = queues_.begin();
    while ((i = std::find_if(i, queues_.end(), [](const auto& current) -> bool
    {
        return current.second->Count() == 0;
    })) != queues_.end())
        queues_.erase(i++);

    // Update the rest
    for (const auto& q : queues_)
    {
        q.second->Update(timeElapsed, [this](const std::string& playerUuid) {
            // These players enter a match now, we should remove them from players_
            auto it = players_.find(playerUuid);
            if (it != players_.end())
                players_.erase(it);
        });
    }
}
