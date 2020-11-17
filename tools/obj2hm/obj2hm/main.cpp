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
    cli.push_back({ "input", { }, "Input OBJ file",
        true, true, sa::arg_parser::option_type::string });
    cli.push_back({ "output", { "-o", "--output" }, "Output PNG file",
        false, true, sa::arg_parser::option_type::string });
}

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << sa::arg_parser::get_help("obj2hm", _cli, "Construct height map image from 3D mesh");
}

static void CreateImage(const Math::Shape& shape, const std::string& filename, int comps)
{
    int width = 0;
    int height = 0;
    float minHeight = 0.0f;
    float maxHeight = 0.0f;
    ea::vector<float> heights = Math::CreateHeightMapFromMesh(shape, width, height, minHeight, maxHeight);

    const float zD = maxHeight - minHeight;

    unsigned char* data = (unsigned char*)malloc((size_t)width * (size_t)height * (size_t)comps);
    memset(data, 0, (size_t)width * (size_t)height * (size_t)comps);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const size_t index = (size_t)y * (size_t)width + (size_t)x;
            float value = heights[index];
            if (!Math::Equals(value, -Math::M_INFINITE))
            {
                unsigned char heightValue = static_cast<unsigned char>(((value - minHeight) / zD) * 255.0f);

                data[index * comps] = heightValue;
                if (comps > 1)
                    data[(index * comps) + 1] = heightValue;
                if (comps > 2)
                    data[(index * comps) + 2] = heightValue;
            }
        }
    }

    stbi_write_png(filename.c_str(), width, height, comps, data, width * comps);

    std::cout << "Created " << filename << " width " << width << " height " << height << std::endl;
    free(data);
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
    std::string ext = Utils::GetFileExt(inputFile);

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
    else
    {
        std::cerr << "Unknown file type " << inputFile << std::endl;
        return 1;
    }

    std::cout << "Shape " << shape.vertexCount_ << " vertices " << shape.indexCount_ << " indices " <<
        shape.GetTriangleCount() << " tris" << std::endl;
    if (!shape.IsTriangles())
    {
        std::cerr << "Shape does not consist of triangles" << std::endl;
        return 1;
    }

    std::string output = sa::arg_parser::get_value<std::string>(parsedArgs, "output", inputFile + ".png");

    CreateImage(shape, output, comps);
    return 0;
}
