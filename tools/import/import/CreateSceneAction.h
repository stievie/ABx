/**
 * Copyright 2020 Stefan Ascher
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

#include <string>
#include <pugixml.hpp>
#include <absmath/Vector3.h>
#include <vector>
#include <memory>
#include <absmath/Shape.h>
#include <absmath/Point.h>

class CreateSceneAction
{
private:
    std::string file_;
    std::string outputDirectory_;
    std::string heightfieldFile_;
    std::string obstaclesHeightmap_;
    Math::Vector3 heightmapSpacing_{ 1.0f, 0.25f, 1.0f };
    int patchSize_{ 32 };
    Math::IntVector2 numPatches_;
    Math::IntVector2 numVertices_;
    Math::Vector2 patchWorldSize_;
    Math::Vector2 patchWorldOrigin_;
    float minHeight_{ -Math::M_INFINITE };
    float maxHeight_{ Math::M_INFINITE };
    std::string navmeshFile_{ "navmesh.bin" };
    std::vector<std::string> searchpaths_;
    std::vector<std::unique_ptr<Math::Shape>> obstackles_;
    int heightmapWidth_{ 0 };
    int heightmapHeight_{ 0 };
    bool LoadScene();
    bool LoadSceneNode(const pugi::xml_node& node, const Math::Matrix4& parentMatrix);
    bool CopySceneFile();
    bool SaveObstacles();
    bool SaveObstaclesObj();
    bool SaveObstaclesHm();
    bool CreateHightmap();
    bool CreateNavMesh();
    bool CreateIndexFile();
    bool CreateTerrainFile();
    bool CreateClientHeightmap();
    bool SaveModel(const Math::Shape& shape, const std::string& filename);
    std::string FindFile(const std::string& name);
public:
    CreateSceneAction(const std::string& file, const std::string& outDir) :
        file_(file),
        outputDirectory_(outDir)
    { }
    ~CreateSceneAction() = default;
    void Execute();

    std::string dataDir_;
    bool createObjs_{ false };
};
