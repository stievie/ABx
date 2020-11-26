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
#include <absmath/Vector3.h>
#include <pugixml.hpp>
#include "TerrainPatch.h"
#include <eastl.hpp>
#include "Scene.h"

namespace Game {

class Game;

/// Holds all the map data, static objects, NavMesh.
class Map
{
private:
    ea::weak_ptr<Game> game_;
    // TerrainPatches are also owned by the game
    ea::vector<ea::shared_ptr<TerrainPatch>> patches_;
    SpawnPoint CorrectedSpanwPoint(const SpawnPoint& world) const;
public:
    Map(ea::shared_ptr<Game> game);
    Map() = delete;
    ~Map();

    void CreatePatches();
    /// Return patch by index.
    TerrainPatch* GetPatch(size_t index) const;
    /// Return patch by patch coordinates.
    TerrainPatch* GetPatch(int x, int z) const;
    size_t GetPatchesCount() const
    {
        return patches_.size();
    }
    float GetTerrainHeight(const Math::Vector3& world) const;
    void UpdatePointHeight(Math::Vector3& world) const;
    ea::shared_ptr<Game> GetGame()
    {
        return game_.lock();
    }

    void AddGameObject(ea::shared_ptr<GameObject> object);
    void UpdateOctree(uint32_t delta);
    SpawnPoint GetFreeSpawnPoint();
    SpawnPoint GetFreeSpawnPoint(const std::string& group);
    SpawnPoint GetFreeSpawnPoint(const ea::vector<SpawnPoint>& points);
    SpawnPoint GetSpawnPoint(const std::string& group) const;
    ea::vector<SpawnPoint> GetSpawnPoints(const std::string& group);

    /// Find a path between world space points. Return non-empty list of points if successful.
    /// Extents specifies how far off the navigation mesh the points can be.
    bool FindPath(ea::vector<Math::Vector3>& dest, const Math::Vector3& start, const Math::Vector3& end,
        const Math::Vector3& extends = Math::Vector3::One, const dtQueryFilter* filter = nullptr);
    Math::Vector3 FindNearestPoint(const Math::Vector3& point, const Math::Vector3& extents,
        const dtQueryFilter* filter = nullptr, dtPolyRef* nearestRef = nullptr);
    bool CanStepOn(const Math::Vector3& point, const Math::Vector3& extents = Math::Vector3::One,
        const dtQueryFilter* filter = nullptr, dtPolyRef* nearestRef = nullptr);

    std::string name_;
    std::string directory_;
    ea::shared_ptr<Navigation::NavigationMesh> navMesh_;
    ea::shared_ptr<Terrain> terrain_;
    ea::unique_ptr<Math::Octree> octree_;
    ea::shared_ptr<Scene> scene_;
};

}
