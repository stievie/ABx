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

#include <absmath/IO.h>
#include <sa/Compiler.h>
#include <iostream>
#include <absmath/VectorMath.h>
#include <sa/ArgParser.h>
#include <absmath/HeightMapTools.h>
#include <abscommon/StringUtils.h>
#include <fstream>

PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4244 4456)
PRAGMA_WARNING_DISABLE_GCC("-Wimplicit-fallthrough=0")
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION
PRAGMA_WARNING_POP

static void InitCli(sa::arg_parser::cli& cli)
{
    cli.push_back({ "help", { "-h", "--help", "-?" }, "Show help",
        false, false, sa::arg_parser::option_type::none });
    cli.push_back({ "comps", { "-c", "--components" }, "Number of color components, default 1",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "targetwidth", { "-W", "--target-width" }, "Target image width",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "targetheight", { "-H", "--target-height" }, "Target image height",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "minheight", { "-minh", "--min-height" }, "Min height value for PNG images (default min height of source)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "maxheight", { "-maxh", "--max-height" }, "Max height value for PNG images (default max height of source)",
        false, true, sa::arg_parser::option_type::number });

    cli.push_back({ "nvx", { "-nvx", "--number-vertices-x" }, "Number of vertices (.hm only)",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "nvy", { "-nvy", "--number-vertices-y" }, "Number of vertices (.hm only)",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "ps", { "-ps", "--patch-size" }, "Patch size (.hm only)",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "pwsx", { "-pwsx", "--patch-world-size-x" }, "Patch world size (.hm only)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "pwsy", { "-pwsy", "--patch-world-size-y" }, "Patch world size (.hm only)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "npx", { "-npx", "--number-patches-x" }, "Number of patches (.hm only)",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "npy", { "-npy", "--number-patches-y" }, "Number of patches (.hm only)",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "pwox", { "-pwox", "--patch-world-origin-x" }, "Patch world origin (.hm only)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "pwoy", { "-pwoy", "--patch-world-origin-y" }, "Patch world origin (.hm only)",
        false, true, sa::arg_parser::option_type::number });

    cli.push_back({ "output", { "-o", "--output" }, "Output file, either .png or .hm",
        true, true, sa::arg_parser::option_type::string });
    cli.push_back({ "input", { }, "Input .obj, .obstacles or .hm file",
        true, true, sa::arg_parser::option_type::string });
}

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << sa::arg_parser::get_help("obj2hm", _cli, "Construct height map from 3D mesh");
}

static void ShowInfo()
{
    std::cout << "obj2h - Construct height map image from 3D mesh" << std::endl;
    std::cout << "(C) 2020, Stefan Ascher" << std::endl << std::endl;
}

static void CreateImage(const ea::vector<float>& heights, const std::string& filename, int comps, int sizeX, int sizeY,
    float minHeight, float maxHeight)
{
    const float zD = maxHeight - minHeight;

    unsigned char* data = (unsigned char*)malloc((size_t)sizeX * (size_t)sizeY * (size_t)comps);
    memset(data, 0, (size_t)sizeX * (size_t)sizeY * (size_t)comps);

    for (int y = 0; y < sizeY; ++y)
    {
        for (int x = 0; x < sizeX; ++x)
        {
            const size_t index = (size_t)y * (size_t)sizeX + (size_t)x;
            float value = heights[index];
            if (!Math::Equals(value, -Math::M_INFINITE))
            {
                unsigned char heightValue = static_cast<unsigned char>(((value - minHeight) / zD) * 255.0f);

                const size_t _index = ((size_t)sizeY - (size_t)y - 1) * (size_t)sizeX + (size_t)x;
                data[_index * comps] = heightValue;
                if (comps > 1)
                    data[(_index * comps) + 1] = heightValue;
                if (comps > 2)
                    data[(_index * comps) + 2] = heightValue;
            }
        }
    }

    stbi_write_png(filename.c_str(), sizeX, sizeY, comps, data, sizeX * comps);

    std::cout << "Created PNG heightmap " << filename << " width " << sizeX << " height " << sizeY << std::endl;
    free(data);
}

static void CreateImage(const Math::Shape& shape, const std::string& filename, int comps, int sizeX, int sizeY,
    float minHeight, float maxHeight)
{
    int width = 0;
    int height = 0;
    float minh = std::numeric_limits<float>::max();
    float maxh = std::numeric_limits<float>::lowest();
    const ea::vector<float> heights = Math::CreateHeightMapFromMesh(shape, sizeX, sizeY, width, height, minh, maxh);
    minHeight = std::min(minh, minHeight);
    maxHeight = std::max(maxh, maxHeight);
    CreateImage(heights, filename, comps, width, height, minHeight, maxHeight);
}

static void CreateHeightmap(const Math::Shape& shape, const std::string& filename, int sizeX, int sizeY,
    const Math::Point<int>& numVertices, int patchSize, const Math::Point<float>& patchWorldSize,
    const Math::Point<int>& numPatches, const Math::Point<float>& patchWorldOrigin)
{
    int width = 0; int height = 0;
    float maxHeight = 0.0f; float minHeight = 0.0f;
    ea::vector<float> heights = Math::CreateHeightMapFromMesh(shape, sizeX, sizeY, width, height, minHeight, maxHeight);

    std::cout << "Num vertices: " << numVertices << ", patch size: " << patchSize <<
        ", patch world size: " << patchWorldSize << ", num patches: " << numPatches <<
        ", patch world origin: " << patchWorldOrigin << ", min/max height: " << minHeight << "/" << maxHeight <<
        ", n height values: " << heights.size() << std::endl;

    std::fstream output(filename, std::ios::binary | std::fstream::out);
    output.write((char*)"HM\0\0", 4);
    output.write((char*)&numVertices.x_, sizeof(int32_t));
    output.write((char*)&numVertices.y_, sizeof(int32_t));

    output.write((char*)&patchSize, sizeof(int32_t));

    output.write((char*)&patchWorldSize.x_, sizeof(float));
    output.write((char*)&patchWorldSize.y_, sizeof(float));
    output.write((char*)&numPatches.x_, sizeof(int32_t));
    output.write((char*)&numPatches.y_, sizeof(int32_t));
    output.write((char*)&patchWorldOrigin.x_, sizeof(float));
    output.write((char*)&patchWorldOrigin.y_, sizeof(float));

    output.write((char*)&minHeight, sizeof(float));
    output.write((char*)&maxHeight, sizeof(float));

    uint32_t c = (uint32_t)heights.size();
    output.write((char*)&c, sizeof(uint32_t));
    output.write((char*)heights.data(), (size_t)c * sizeof(float));

    output.close();
}

static bool LoadHeightmap(const std::string& filename, ea::vector<float>& heights,
    int& patchSize,
    Math::Point<float>& patchWorldSize, Math::Point<int>& numPatches,
    Math::Point<int>& numVertices, Math::Point<float>& patchWorldOrigin,
    float& minHeight, float& maxHeight)
{
    heights = IO::LoadHeightmap(filename, patchSize, patchWorldSize, numPatches, numVertices,
        patchWorldOrigin, minHeight, maxHeight);
    return heights.size() != 0;
}

static bool LoadObstacles(const std::string& filename, Math::Shape& shape)
{
    std::fstream input(filename, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    size_t obstacleCount = 0;
    input.read((char*)&obstacleCount, sizeof(uint64_t));
    for (size_t obstacle = 0; obstacle < obstacleCount; ++obstacle)
    {
        unsigned offset = (unsigned)shape.vertexCount_;
        size_t vertexCount = 0;
        input.read((char*)&vertexCount, sizeof(uint64_t));
        for (size_t vertex = 0; vertex < vertexCount; ++vertex)
        {
            float x = 0.0f, y = 0.0f, z = 0.0f;
            input.read((char*)&x, sizeof(float));
            input.read((char*)&y, sizeof(float));
            input.read((char*)&z, sizeof(float));
            shape.vertexData_.push_back({ x, y, z });
            ++shape.vertexCount_;
        }

        size_t indexCount = 0;
        input.read((char*)&indexCount, sizeof(uint64_t));
        for (size_t index = 0; index < indexCount; index += 3)
        {
            int i1 = 0, i2 = 0, i3 = 0;
            input.read((char*)&i1, sizeof(int));
            input.read((char*)&i2, sizeof(int));
            input.read((char*)&i3, sizeof(int));
            shape.AddTriangle(offset + i1, offset + i2, offset + i3);
        }
    }
    return true;
}

int main(int argc, char** argv)
{
    ShowInfo();
    sa::arg_parser::cli _cli;
    InitCli(_cli);
    sa::arg_parser::values parsedArgs;
    sa::arg_parser::result cmdres = sa::arg_parser::parse(argc, argv, _cli, parsedArgs);
    auto val = sa::arg_parser::get_value<bool>(parsedArgs, "help");
    if (val.has_value() && val.value())
    {
        ShowHelp(_cli);
        return 0;
    }
    if (!cmdres)
    {
        std::cout << cmdres << std::endl;
        ShowHelp(_cli);
        return 1;
    }

    auto actval = sa::arg_parser::get_value<std::string>(parsedArgs, "0");
    if (!actval.has_value())
    {
        std::cerr << "No input file provided" << std::endl;
        return 1;
    }
    int comps = sa::arg_parser::get_value<int>(parsedArgs, "comps", 1);
    if (comps < 1 || comps > 3)
    {
        std::cerr << "Components must between 1 and 3" << std::endl;
        return 1;
    }

    std::string inputFile = actval.value();
    Math::Shape shape;
    ea::vector<float> heights;
    std::string ext = Utils::GetFileExt(inputFile);

    Math::Point<int> numVertices;
    int patchSize = 0;
    Math::Point<float> patchWorldSize;
    Math::Point<int> numPatches;
    Math::Point<float> patchWorldOrigin;
    float minHeight = 0.0f, maxHeight = 0.0f;

    if (Utils::StringEquals(ext, ".obj"))
    {
        if (!IO::LoadShapeFromOBJ(inputFile, shape))
        {
            std::cerr << "Error loading " << inputFile << std::endl;
            return 1;
        }
    }
    else if (Utils::StringEquals(ext, ".obstacles"))
    {
        if (!LoadObstacles(inputFile, shape))
        {
            std::cerr << "Error loading " << inputFile << std::endl;
            return 1;
        }
    }
    else if (Utils::StringEquals(ext, ".hm"))
    {
        if (!LoadHeightmap(inputFile, heights, patchSize, patchWorldSize, numPatches, numVertices, patchWorldOrigin,
            minHeight, maxHeight))
        {
            std::cerr << "Error loading " << inputFile << std::endl;
            return 1;
        }
    }
    else
    {
        std::cerr << "Unknown file type " << inputFile << std::endl;
        return 1;
    }

    if (shape.vertexCount_ != 0)
    {
        std::cout << "Shape " << shape.vertexCount_ << " vertices " << shape.indexCount_ << " indices " <<
            shape.GetTriangleCount() << " tris" << std::endl;
        if (!shape.IsTriangles())
        {
            std::cerr << "Shape does not consist of triangles" << std::endl;
            return 1;
        }
    }

    std::string output = sa::arg_parser::get_value<std::string>(parsedArgs, "output", inputFile + ".png");
    int sizeX = sa::arg_parser::get_value<int>(parsedArgs, "targetwidth", 0);
    int sizeY = sa::arg_parser::get_value<int>(parsedArgs, "targetheight", 0);
    std::string outext = Utils::GetFileExt(output);
    if (Utils::StringEquals(outext, ".png"))
    {
        float minH = sa::arg_parser::get_value<float>(parsedArgs, "minheight", std::numeric_limits<float>::max());
        float maxH = sa::arg_parser::get_value<float>(parsedArgs, "maxheight", std::numeric_limits<float>::lowest());
        minHeight = std::min(minH, minHeight);
        maxHeight = std::max(maxH, maxHeight);
        if (heights.size() != 0)
        {
            CreateImage(heights, output, comps, sizeX, sizeY, minHeight, maxHeight);
            return 0;
        }
        else if (shape.vertexCount_ != 0)
        {
            CreateImage(shape, output, comps, sizeX, sizeY, minHeight, maxHeight);
            return 0;
        }
    }

    if (shape.vertexCount_ == 0)
    {
        std::cerr << "Shape does not have vertices. Note: Can not create .hm from .hm" << std::endl;
        return 1;
    }
    numVertices.x_ = sa::arg_parser::get_value<int>(parsedArgs, "nvx", 0);
    numVertices.y_ = sa::arg_parser::get_value<int>(parsedArgs, "nvy", 0);
    if (numVertices.x_ == 0 || numVertices.y_ == 0)
    {
        std::cerr << "Missing argument number of vertices" << std::endl;
        return 1;
    }

    patchSize = sa::arg_parser::get_value<int>(parsedArgs, "ps", 0);
    if (patchSize == 0)
    {
        std::cerr << "Missing argument patch size" << std::endl;
        return 1;
    }

    patchWorldSize.x_ = sa::arg_parser::get_value<float>(parsedArgs, "pwsx", 0.0f);
    patchWorldSize.y_ = sa::arg_parser::get_value<float>(parsedArgs, "pwsy", 0.0f);
    if (Math::Equals(patchWorldSize.x_, 0.0f) || Math::Equals(patchWorldSize.y_, 0.0f))
    {
        std::cerr << "Missing argument patch world size" << std::endl;
        return 1;
    }

    numPatches.x_ = sa::arg_parser::get_value<int>(parsedArgs, "npx", 0);
    numPatches.y_ = sa::arg_parser::get_value<int>(parsedArgs, "npy", 0);
    if (numPatches.x_ == 0 || numPatches.y_ == 0)
    {
        std::cerr << "Missing argument number of patches" << std::endl;
        return 1;
    }

    patchWorldOrigin.x_ = sa::arg_parser::get_value<float>(parsedArgs, "pwox", Math::M_INFINITE);
    patchWorldOrigin.y_ = sa::arg_parser::get_value<float>(parsedArgs, "pwoy", Math::M_INFINITE);
    if (Math::IsInfinite(patchWorldOrigin.x_) || Math::IsInfinite(patchWorldOrigin.y_))
    {
        std::cerr << "Missing argument patch world origin" << std::endl;
        return 1;
    }

    CreateHeightmap(shape, output, sizeX, sizeY,
        numVertices, patchSize, patchWorldSize, numPatches, patchWorldOrigin);
    std::cout << "Created HM heightmap " << output << std::endl;
    return 0;
}
