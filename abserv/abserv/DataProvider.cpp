#include "stdafx.h"
#include "DataProvider.h"
#include "ConfigManager.h"
#include "NavigationMesh.h"
#include "IONavMesh.h"

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
    if (cache_.size() == 0)
        return;

    for (auto it = cache_.begin(); it != cache_.end(); )
    {
        if ((*it).second.use_count() == 1)
        {
            cache_.erase(it);
            if (cache_.size() == 0)
                return;
        }
        else
            it++;
    }
}

}