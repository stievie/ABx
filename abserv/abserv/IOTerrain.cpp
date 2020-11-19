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

#include "IOTerrain.h"
#include <fstream>
#include <sa/StringTempl.h>
#include <pugixml.hpp>
#include "DataProvider.h"

namespace IO {

bool IOTerrain::Import(Game::Terrain& asset, const std::string& name)
{
    pugi::xml_document doc;
    const pugi::xml_parse_result result = doc.load_file(name.c_str());
    if (result.status != pugi::status_ok)
    {
        LOG_ERROR << "Error loading file " << name << ": " << result.description() << std::endl;
        return false;
    }
    const pugi::xml_node indexNode = doc.child("terrain");
    if (!indexNode)
    {
        LOG_ERROR << "File " << name << " does not have a terrain node" << std::endl;
        return false;
    }

    auto* dataProv = GetSubsystem<IO::DataProvider>();

    using namespace sa::literals;
    // Let's just assume heightmap files are relative to terrain files
    std::string dir = Utils::ExtractFileDir(name);
    for (const auto& fileNode : indexNode.children("file"))
    {
        const pugi::xml_attribute& typeAttr = fileNode.attribute("type");
        const size_t typeHash = sa::StringHashRt(typeAttr.as_string());
        const pugi::xml_attribute& srcAttr = fileNode.attribute("src");
        switch (typeHash)
        {
        case "Heightmap.0"_Hash:
            if (!asset.GetHeightMap())
                asset.SetHeightMap(dataProv->GetAsset<Game::HeightMap>(Utils::ConcatPath(dir, srcAttr.as_string())));
            break;
        case "Heightmap.1"_Hash:
            if (!asset.GetHeightMap2())
                asset.SetHeightMap2(dataProv->GetAsset<Game::HeightMap>(Utils::ConcatPath(dir, srcAttr.as_string())));
            break;
        }
    }
    // We need at least one heightmap
    auto* hm1 = asset.GetHeightMap();
    if (!hm1)
        return false;
    auto* hm2 = asset.GetHeightMap2();
    if (hm1 && hm2)
    {
        if (hm1->heightData_.size() != hm2->heightData_.size())
        {
            LOG_WARNING << "Height map layers have different sizes, " << hm1->heightData_.size() <<
                " != " << hm2->heightData_.size() << std::endl;
        }
    }
    asset.Initialize();

    return true;
}

}
