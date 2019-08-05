#pragma once

#include "Queue.h"

class MatchQueues
{
private:
    std::map<std::string, std::unique_ptr<Queue>> queues_;
    std::unordered_map<std::string, std::string> parties_;
    std::mutex lock_;
public:
    void Add(const std::string& mapUuid, const std::string& playerUuid);
    void Remove(const std::string& playerUuid);
    Queue* GetQueue(const std::string& mapUuid);
    void Update(uint32_t timeElapsed);
};
