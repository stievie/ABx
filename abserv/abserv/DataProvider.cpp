#include "stdafx.h"
#include "DataProvider.h"
#include "ConfigManager.h"
#include "NavigationMesh.h"
#include "IONavMesh.h"
#include "Scheduler.h"

namespace IO {

DataProvider DataProvider::Instance;

DataProvider::DataProvider()
{
    // Add Importer
    IO::DataProvider::Instance.AddImporter<Game::NavigationMesh, IO::IONavMesh>();
}

void DataProvider::CleanCacheTask()
{
    CleanCache();
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&DataProvider::CleanCacheTask, this))
    );
}

void DataProvider::Run()
{
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&DataProvider::CleanCacheTask, this))
    );
}

std::string DataProvider::GetDataFile(const std::string& name) const
{
    static std::string dataDir = ConfigManager::Instance[ConfigManager::DataDir].GetString();
    return dataDir + name;
}

bool DataProvider::FileExists(const std::string& name)
{
    return Utils::FileExists(name);
}

std::string DataProvider::GetFile(const std::string & name) const
{
    if (Utils::FileExists(name))
        return name;
    std::string n = GetDataFile(name);
    if (Utils::FileExists(n))
        return n;
    return name;
}

void DataProvider::CleanCache()
{
    for (auto it = cache_.begin(); it != cache_.end(); )
    {
        if ((*it).second.use_count() == 1)
            cache_.erase(it);
        else
            it++;
    }
}

}