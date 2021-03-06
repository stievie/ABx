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

#include <sa/ArgParser.h>
#include <iostream>
#include <sa/Compiler.h>
#include <absmath/HeightMapTools.h>
#include <abscommon/StringUtils.h>
#include <absmath/IO.h>
#include <fstream>
#include <absmath/MathDefs.h>

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

Math::Vector3 scaling = { 1.0f, 0.25f, 1.0f };
int patchSize = 32;

static void InitCli(sa::arg_parser::cli& cli)
{
    cli.push_back({ "help", { "-h", "--help", "-?" }, "Show help",
        false, false, sa::arg_parser::option_type::none });
    cli.push_back({ "red", { "-R", "--red" }, "Red channel",
        true, true, sa::arg_parser::option_type::string });
    cli.push_back({ "green", { "-G", "--green" }, "Creen channel",
        false, true, sa::arg_parser::option_type::string });
    cli.push_back({ "blue", { "-B", "--blue" }, "Blue channel",
        false, true, sa::arg_parser::option_type::string });
    cli.push_back({ "alpha", { "-A", "--alpha" }, "Alpha channel",
        false, true, sa::arg_parser::option_type::string });
    cli.push_back({ "scalex", { "-X", "--scale-x" }, "X scaling (default 1.0)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "scaley", { "-Y", "--scale-y" }, "Y scaling (default 0.25)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "scalez", { "-Z", "--scale-z" }, "Z scaling (default 1.0)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "patchsize", { "-P", "--patch-size" }, "Patch size (default 32)",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "targetwidth", { "-W", "--target-width" }, "Target image width",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "targetheight", { "-H", "--target-height" }, "Target image height",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "output", { "-o", "--output" }, "Output PNG file",
        true, true, sa::arg_parser::option_type::string });
}

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << sa::arg_parser::get_help("hmerge", _cli, "Merge heightmaps");
}

static void ShowInfo()
{
    std::cout << "hmerge - Merge heightmaps" << std::endl;
    std::cout << "(C) 2020, Stefan Ascher" << std::endl << std::endl;
}

static bool GetHeights(const std::string& filename, int targetWidth, int targetHeight,
    ea::vector<float>& heights, int& width, int& height, float& minHeight, float& maxHeight)
{
    std::string ext = Utils::GetFileExt(filename);

    if (Utils::StringEquals(ext, ".png"))
    {
        int comps = 0;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &comps, 0);
        if (!data)
            return false;

        Math::Vector2 patchWorldSize;
        Math::IntVector2 numPatches;
        Math::IntVector2 numVertices;
        Math::Vector2 patchWorldOrigin;
        heights = Math::CreateHeightMapFromImage((const unsigned char*)data, width, height, comps, scaling, patchSize,
            patchWorldSize, numPatches, numVertices, patchWorldOrigin, minHeight, maxHeight);
        free(data);
        return true;
    }

    if (Utils::StringEquals(ext, ".obj"))
    {
        Math::Shape mesh;
        if (!IO::LoadShapeFromOBJ(filename, mesh))
            return false;

        heights = Math::CreateHeightMapFromMesh(mesh, targetWidth, targetHeight, width, height, minHeight, maxHeight);
        return true;
    }
    if (Utils::StringEquals(ext, ".hm"))
    {
        Math::Vector2 patchWorldSize;
        Math::IntVector2 numPatches;
        Math::IntVector2 numVertices;
        Math::Vector2 patchWorldOrigin;
        width = targetWidth;
        height = targetHeight;
        heights = IO::LoadHeightmap(filename, patchSize, patchWorldSize, numPatches, numVertices,
            patchWorldOrigin, minHeight, maxHeight);
#ifdef _DEBUG
        std::cout << "Num vertices: " << numVertices << ", patch size: " << patchSize <<
            ", patch world size: " << patchWorldSize << ", num patches: " << numPatches <<
            ", patch world origin: " << patchWorldOrigin << ", min/max height: " << minHeight << "/" << maxHeight <<
            ", n height values: " << heights.size() << std::endl;
#endif
        return true;
    }
    if (Utils::StringEquals(ext, ".obstacles"))
    {
        std::fstream input(filename, std::ios::binary | std::fstream::in);
        if (!input.is_open())
            return false;

        Math::Shape mesh;
        size_t obstacleCount = 0;
        input.read((char*)&obstacleCount, sizeof(uint64_t));
        for (size_t obstacle = 0; obstacle < obstacleCount; ++obstacle)
        {
            unsigned offset = (unsigned)mesh.vertexCount_;
            size_t vertexCount = 0;
            input.read((char*)&vertexCount, sizeof(uint64_t));
            for (size_t vertex = 0; vertex < vertexCount; ++vertex)
            {
                float x = 0.0f, y = 0.0f, z = 0.0f;
                input.read((char*)&x, sizeof(float));
                input.read((char*)&y, sizeof(float));
                input.read((char*)&z, sizeof(float));
                mesh.vertexData_.push_back({ x, y, z });
                ++mesh.vertexCount_;
            }

            size_t indexCount = 0;
            input.read((char*)&indexCount, sizeof(uint64_t));
            for (size_t index = 0; index < indexCount; index += 3)
            {
                int i1 = 0, i2 = 0, i3 = 0;
                input.read((char*)&i1, sizeof(int));
                input.read((char*)&i2, sizeof(int));
                input.read((char*)&i3, sizeof(int));
                mesh.AddTriangle(offset + i1, offset + i2, offset + i3);
            }
        }
        heights = Math::CreateHeightMapFromMesh(mesh, targetWidth, targetHeight, width, height, minHeight, maxHeight);
        return true;
    }

    std::cerr << "Unknown file type " << filename << std::endl;
    return false;
}

static bool CreateImage(const std::string& filename,
    const ea::vector<float>& red,
    const ea::vector<float>& green,
    const ea::vector<float>& blue,
    const ea::vector<float>& alpha,
    int width, int height,
    float minHeight, float maxHeight
)
{
    // We either have 1, 3 or 4 components
    int comps = 1;
    if (alpha.size() != 0)
        comps = 4;
    else if (blue.size() != 0 || green.size() != 0)
        comps = 3;

    const float zD = maxHeight - minHeight;

    unsigned char* data = (unsigned char*)malloc((size_t)width * (size_t)height * (size_t)comps);
    if (!data)
        return false;

    memset(data, 0, (size_t)width * (size_t)height * (size_t)comps);

    auto setValue = [&](const ea::vector<float>& heights, size_t x, size_t y, size_t offset)
    {
        const size_t index = ((size_t)y * ((size_t)width) + (size_t)x);
        if (heights.size() == 0 || index >= heights.size())
            return;

        const float value = heights[index];
        if (Math::Equals(value, -Math::M_INFINITE))
            return;

        const unsigned char heightValue = static_cast<unsigned char>(((value - minHeight) / zD) * 255.0f);
        const size_t _index = ((size_t)height - y - 1) * (size_t)width + (size_t)x;
        data[(_index * comps) + offset] = heightValue;
    };

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            setValue(red, x, y, 0);
            if (comps > 1)
                setValue(green, x, y, 1);
            if (comps > 2)
                setValue(blue, x, y, 2);
            if (comps > 3)
                setValue(alpha, x, y, 3);
        }
    }

    stbi_write_png(filename.c_str(), width, height, comps, data, width * comps);

    std::cout << "Created " << filename << " width " << width << ", height " << height <<
        ", min/max height " << minHeight << "/" << maxHeight << std::endl;

    free(data);
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

    scaling.x_ = sa::arg_parser::get_value<float>(parsedArgs, "scalex", scaling.x_);
    scaling.y_ = sa::arg_parser::get_value<float>(parsedArgs, "scaley", scaling.y_);
    scaling.z_ = sa::arg_parser::get_value<float>(parsedArgs, "scalez", scaling.z_);
    patchSize = sa::arg_parser::get_value<int>(parsedArgs, "patchsize", patchSize);
    int targetWidth = sa::arg_parser::get_value<int>(parsedArgs, "targetwidth", 0);
    int targetHeight = sa::arg_parser::get_value<int>(parsedArgs, "targetheight", 0);

    std::string redFile = sa::arg_parser::get_value<std::string>(parsedArgs, "red", "");
    if (redFile.empty())
    {
        return 1;
    }
    std::cout << "Red layer " << redFile << std::endl;

    std::string greenFile = sa::arg_parser::get_value<std::string>(parsedArgs, "green", "");
    std::string blueFile = sa::arg_parser::get_value<std::string>(parsedArgs, "blue", "");
    std::string alphaFile = sa::arg_parser::get_value<std::string>(parsedArgs, "alpha", "");

    int width = 0;
    int height = 0;
    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = std::numeric_limits<float>::min();

    ea::vector<float> red;
    ea::vector<float> green;
    ea::vector<float> blue;
    ea::vector<float> alpha;

    int w, h;
    float minH, maxH;

    if (!GetHeights(redFile, targetWidth, targetHeight, red, w, h, minH, maxH))
    {
        std::cerr << "Unable to load file " << redFile << std::endl;
        return 1;
    }
    width = std::max(width, w);
    height = std::max(height, h);
    minHeight = std::min(minHeight, minH);
    maxHeight = std::max(maxHeight, maxH);

    if (!greenFile.empty())
    {
        std::cout << "Green layer " << greenFile << std::endl;
        if (!GetHeights(greenFile, targetWidth, targetHeight, green, w, h, minH, maxH))
        {
            std::cerr << "Unable to load file " << greenFile << std::endl;
            return 1;
        }
        width = std::max(width, w);
        height = std::max(height, h);
        minHeight = std::min(minHeight, minH);
        maxHeight = std::max(maxHeight, maxH);
        if (red.size() != green.size())
        {
            std::cout << "WARNING: Layers have diffrent sizes" << std::endl;
        }
    }
    if (!blueFile.empty())
    {
        std::cout << "Blue layer " << blueFile << std::endl;
        if (!GetHeights(blueFile, targetWidth, targetHeight, blue, w, h, minH, maxH))
        {
            std::cerr << "Unable to load file " << blueFile << std::endl;
            return 1;
        }
        width = std::max(width, w);
        height = std::max(height, h);
        minHeight = std::min(minHeight, minH);
        maxHeight = std::max(maxHeight, maxH);
        if (red.size() != blue.size())
        {
            std::cout << "WARNING: Layers have diffrent sizes" << std::endl;
        }
    }
    if (!alphaFile.empty())
    {
        std::cout << "Alpha layer " << alphaFile << std::endl;
        if (!GetHeights(alphaFile, targetWidth, targetHeight, alpha, w, h, minH, maxH))
        {
            std::cerr << "Unable to load file " << alphaFile << std::endl;
            return 1;
        }
        width = std::max(width, w);
        height = std::max(height, h);
        minHeight = std::min(minHeight, minH);
        maxHeight = std::max(maxHeight, maxH);
        if (red.size() != alpha.size())
        {
            std::cout << "WARNING: Layers have diffrent sizes" << std::endl;
        }
    }

    std::string outfile = sa::arg_parser::get_value<std::string>(parsedArgs, "output", "");
    if (!CreateImage(outfile, red, green, blue, alpha, width, height, minHeight, maxHeight))
    {
        std::cerr << "Error creating image" << std::endl;
        return 1;
    }
    return 0;
}
