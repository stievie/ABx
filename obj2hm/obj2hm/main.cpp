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
    const Math::Vector3 minX = shape.GetFarsetPointInDirection(-Math::Vector3::UnitX);
    const Math::Vector3 maxX = shape.GetFarsetPointInDirection(Math::Vector3::UnitX);
    const Math::Vector3 minZ = shape.GetFarsetPointInDirection(-Math::Vector3::UnitZ);
    const Math::Vector3 maxZ = shape.GetFarsetPointInDirection(Math::Vector3::UnitZ);

    const Math::Vector3 minHeight = shape.GetFarsetPointInDirection(-Math::Vector3::UnitY);
    const Math::Vector3 maxHeight = shape.GetFarsetPointInDirection(Math::Vector3::UnitY);
    const float zD = maxHeight.y_ - minHeight.y_;

    const int width = (int)std::ceil(maxX.x_ - minX.x_);
    const int height = (int)std::ceil(maxZ.z_ - minZ.z_);

    char* data = (char*)malloc((size_t)width * (size_t)height * (size_t)comps);
    memset(data, 0, (size_t)width * (size_t)height * (size_t)comps);

    std::vector<float> heights;
    heights.resize((size_t)width * (size_t)height);
    std::fill(heights.begin(), heights.end(), std::numeric_limits<float>::min());

    for (const auto& v : shape.vertexData_)
    {
        int x = static_cast<int>(v.x_ - minX.x_);
        int y = height - static_cast<int>(v.z_ - minZ.z_);

        if (x >= width)
            continue;
        if (y >= height)
            continue;

        const size_t index = (size_t)y * (size_t)width + (size_t)x;
        if (heights[index] < v.y_)
            heights[index] = v.y_;
    }

    float lastValue = 0.0f;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const size_t index = ((size_t)y * (size_t)width + (size_t)x) * (size_t)comps;
            float value = heights[index];
            if (Math::Equals(value, std::numeric_limits<float>::min()))
                value = lastValue;

            char heightValue = static_cast<char>(((value - minHeight.y_) / zD) * 255.0f);

            data[index] = heightValue;
            if (comps > 1)
                data[index + 1] = heightValue;
            if (comps > 2)
                data[index + 2] = heightValue;
            lastValue = value;
        }
    }

    stbi_write_png(filename.c_str(), width, height, comps, data, width * comps);

    std::cout << "Created " << filename << " width " << width << " height " << height << std::endl;
    free(data);
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
    if (!IO::LoadShapeFromOBJ(inputFile, shape))
        return 1;

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
