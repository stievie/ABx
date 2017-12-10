#include "stdafx.h"
#include "DataProvider.h"
#include "ConfigManager.h"
#include "NavigationMesh.h"
#include "IONavMesh.h"
#include <algorithm>

#include "DebugNew.h"

namespace IO {

DataProvider DataProvider::Instance;

DataProvider::DataProvider()
{
    // Add Importer
    IO::DataProvider::Instance.AddImporter<Game::NavigationMesh, IO::IONavMesh>();
}

std::string DataProvider::GetDataFile(const std::string& name) const
{
    static std::string dataDir = ConfigManager::Instance[ConfigManager::DataDir].GetString();
    return dataDir + name;
}

bool DataProvider::FileExists(const std::string& name) const
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
    if (cache_.size() == 0)
        return;

    // Delete all assets that are only owned by the cache
    auto i = cache_.begin();
    while ((i = std::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        return current.second.use_count() == 1;
    })) != cache_.end())
        cache_.erase(i++);
}

}