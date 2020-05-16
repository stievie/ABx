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

#include "stdafx.h"
#include "DataProvider.h"
#include "ConfigManager.h"
#include "NavigationMesh.h"
#include "IONavMesh.h"
#include "IOTerrain.h"
#include "Terrain.h"
#include "Model.h"
#include "IOModel.h"
#include "IOScript.h"

namespace IO {

DataProvider::DataProvider()
{
    // Add Importer
    AddImporter<Navigation::NavigationMesh, IO::IONavMesh>();
    AddImporter<Game::Terrain, IO::IOTerrain>();
    AddImporter<Game::Model, IO::IOModel>();
    AddImporter<Game::Script, IO::IOScript>();
}

std::string DataProvider::GetDataFile(const std::string& name) const
{
    static std::string dataDir = (*GetSubsystem<ConfigManager>())[ConfigManager::DataDir].GetString();
    return dataDir + name;
}

const std::string& DataProvider::GetDataDir() const
{
    return (*GetSubsystem<ConfigManager>())[ConfigManager::DataDir].GetString();
}

bool DataProvider::FileExists(const std::string& name) const
{
    return Utils::FileExists(name);
}

std::string DataProvider::GetFile(const std::string& name) const
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

#ifdef _DEBUG
    LOG_DEBUG << "Cleaning cache" << std::endl;
#endif
    // Delete all assets that are only owned by the cache
    auto i = cache_.begin();
    while ((i = ea::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        return current.second.use_count() == 1;
    })) != cache_.end())
        cache_.erase(i++);
}

}
