#pragma once

#include "Queue.h"

class MatchQueues
{
private:
    std::map<std::string, std::unique_ptr<Queue>> queues_;
    /// Player -> Map to find the queue when the players wants to be removed from it
    std::unordered_map<std::string, std::string> players_;
    std::mutex lock_;
public:
    void Add(const std::string& mapUuid, const std::string& playerUuid);
    void Remove(const std::string& playerUuid);
    Queue* GetQueue(const std::string& mapUuid);
    void Update(uint32_t timeElapsed);
};
