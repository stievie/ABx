#include "stdafx.h"
#include "Bans.h"
#include "Utils.h"
#include "ConfigManager.h"

BanManager BanManager::Instance;

bool BanManager::AcceptConnection(uint32_t clientIP)
{
    if (clientIP == 0)
        return false;

    lock_.lock();

    uint64_t currentTime = Utils::AbTick();
    std::map<uint32_t, ConnectBlock>::iterator it = ipConnects_.find(clientIP);
    if (it == ipConnects_.end())
    {
        ipConnects_.emplace(clientIP, ConnectBlock(currentTime, 0, 1));
        lock_.unlock();
        return true;
    }

    ConnectBlock& cb = it->second;
    cb.count++;
    if (cb.blockTime > currentTime)
    {
        lock_.unlock();
        return false;
    }

    if (currentTime - cb.startTime > 1000)
    {
        uint32_t connectionsPerSec = cb.count;
        cb.startTime = currentTime;
        cb.count = 0;
        cb.blockTime = 0;

        if (connectionsPerSec > 10)
        {
            cb.blockTime = currentTime + 10000;
            lock_.unlock();
            return false;
        }
    }

    lock_.unlock();
    return true;
}
