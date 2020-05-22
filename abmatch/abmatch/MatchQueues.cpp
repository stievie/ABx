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

#include "MatchQueues.h"

void MatchQueues::Add(const std::string& mapUuid, const std::string& playerUuid)
{
    std::scoped_lock lock(lock_);
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
    std::scoped_lock lock(lock_);
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
        auto queue = ea::make_unique<Queue>(mapUuid);
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
    std::scoped_lock lock(lock_);
    // Delete empty queues
    auto i = queues_.begin();
    while ((i = ea::find_if(i, queues_.end(), [](const auto& current) -> bool
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
