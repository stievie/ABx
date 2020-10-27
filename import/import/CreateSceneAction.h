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

class CreateSceneAction
{
private:
    std::string file_;
    std::string outputDirectory_;
    std::string heightfieldFile_;
    Math::Vector3 heightmapSpacing_{ 1.0f, 0.25f, 1.0f };
    int patchSize_{ 32 };
    std::string navmeshFile_{ "navmesh.bin" };
    std::vector<std::string> searchpaths_;
    std::vector<std::unique_ptr<Math::Shape>> obstackles_;
    bool LoadScene();
    bool LoadSceneNode(const pugi::xml_node& node);
    bool CopySceneFile();
    bool SaveObstacles();
    bool CreateHightmap();
    bool CreateNavMesh();
    bool CreateIndexFile();
    std::string FindFile(const std::string& name);
public:
    CreateSceneAction(const std::string& file, const std::string& outDir) :
        file_(file),
        outputDirectory_(outDir)
    { }
    ~CreateSceneAction() = default;
    void Execute();
};
