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

#pragma once

#include "NavigationMesh.h"
#include "Octree.h"
#include "Terrain.h"
#include "Vector3.h"
#include <pugixml.hpp>
#include "TerrainPatch.h"

namespace Game {

class Game;

/// Database map data
struct MapData
{
    /// DB ID
    uint32_t id;
    /// The name of the map
    std::string name;
    std::string directory;
};

struct SpawnPoint
{
    Math::Vector3 position;
    Math::Quaternion rotation;
    std::string group;
    // equality comparison. doesn't modify object. therefore const.
    bool operator==(const SpawnPoint& rhs) const
    {
        return (position == rhs.position && rotation == rhs.rotation);
    }
    inline bool Empty() const;
};

static const SpawnPoint EmtpySpawnPoint{ Math::Vector3::Zero, Math::Quaternion::Identity, "" };
inline bool SpawnPoint::Empty() const
{
    return *this == EmtpySpawnPoint;
}


/// Holds all the map data, static objects, NavMesh.
class Map
{
private:
    std::weak_ptr<Game> game_;
    // TerrainPatches are also owned by the game
    std::vector<std::shared_ptr<TerrainPatch>> patches_;
public:
    Map(std::shared_ptr<Game> game);
    Map() = delete;
    ~Map();

    void CreatePatches();
    /// Return patch by index.
    TerrainPatch* GetPatch(unsigned index) const;
    /// Return patch by patch coordinates.
    TerrainPatch* GetPatch(int x, int z) const;
    size_t GetPatchesCount() const
    {
        return patches_.size();
    }
    float GetTerrainHeight(const Math::Vector3& world) const;
    void UpdatePointHeight(Math::Vector3& world) const;
    std::shared_ptr<Game> GetGame()
    {
        return game_.lock();
    }

    void AddGameObject(std::shared_ptr<GameObject> object);
    void UpdateOctree(uint32_t delta);
    SpawnPoint GetFreeSpawnPoint();
    SpawnPoint GetFreeSpawnPoint(const std::string& group);
    SpawnPoint GetFreeSpawnPoint(const std::vector<SpawnPoint>& points);
    const SpawnPoint& GetSpawnPoint(const std::string& group) const;
    std::vector<SpawnPoint> GetSpawnPoints(const std::string& group);

    /// Find a path between world space points. Return non-empty list of points if successful.
    /// Extents specifies how far off the navigation mesh the points can be.
    bool FindPath(std::vector<Math::Vector3>& dest, const Math::Vector3& start, const Math::Vector3& end,
        const Math::Vector3& extends = Math::Vector3::One, const dtQueryFilter* filter = nullptr);
    Math::Vector3 FindNearestPoint(const Math::Vector3& point, const Math::Vector3& extents,
        const dtQueryFilter* filter = nullptr, dtPolyRef* nearestRef = nullptr);

    MapData data_;
    std::vector<SpawnPoint> spawnPoints_;
    std::shared_ptr<Navigation::NavigationMesh> navMesh_;
    std::shared_ptr<Terrain> terrain_;
    std::unique_ptr<Math::Octree> octree_;
};

}
