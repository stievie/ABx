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
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4244 4456)
PRAGMA_WARNING_DISABLE_GCC("-Wimplicit-fallthrough=0")
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION
PRAGMA_WARNING_POP
#include <absmath/HeightMapTools.h>
#include <absmath/IO.h>

static void InitCli(sa::arg_parser::cli& cli)
{
    cli.push_back({ "help", { "-h", "--help", "-?" }, "Show help",
        false, false, sa::arg_parser::option_type::none });

    cli.push_back({ "spacingx", { "-X", "--spacing-x" }, "Spacing X (default 1)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "spacingy", { "-Y", "--spacing-y" }, "Spacing Y (default 0.2)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "spacingz", { "-Z", "--spacing-z" }, "Spacing Z (default 1)",
        false, true, sa::arg_parser::option_type::number });
    cli.push_back({ "patchsize", { "-P", "--patch-size" }, "Patch size (default 32)",
        false, true, sa::arg_parser::option_type::integer });

    cli.push_back({ "output", { "-o", "--output" }, "Output OBJ file",
        false, true, sa::arg_parser::option_type::string });
    cli.push_back({ "input", { }, "Input PNG file",
        true, true, sa::arg_parser::option_type::string });
}

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << sa::arg_parser::get_help("hm2obj", _cli, "Construct 3D mesh from heightmap");
}

static void ShowInfo()
{
    std::cout << "hm2obj - Construct 3D mesh from heightmap" << std::endl;
    std::cout << "(C) 2020, Stefan Ascher" << std::endl << std::endl;
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

    auto inputval = sa::arg_parser::get_value<std::string>(parsedArgs, "0");
    if (!inputval.has_value())
    {
        std::cerr << "No input file provided" << std::endl;
        return 1;
    }
    std::string inputFile = inputval.value();
    std::string outputFile = sa::arg_parser::get_value<std::string>(parsedArgs, "output", inputFile + ".obj");

    int width = 0; int height = 0; int components = 0;
    unsigned char* data = stbi_load(inputFile.c_str(), &width, &height, &components, 0);
    if (!data)
    {
        std::cout << "Error loading file " << inputFile << std::endl;
        return 1;
    }

    Math::Vector3 spacing;
    spacing.x_ = sa::arg_parser::get_value<float>(parsedArgs, "spacingx", 1.0f);
    spacing.y_ = sa::arg_parser::get_value<float>(parsedArgs, "spacingy", 0.2f);
    spacing.z_ = sa::arg_parser::get_value<float>(parsedArgs, "spacingz", 1.0f);
    int patchSize = sa::arg_parser::get_value<int>(parsedArgs, "patchsize", 32);

    Math::Vector2 patchWorldSize;
    Math::IntVector2 numPatches;
    Math::IntVector2 numVertices;
    Math::Vector2 patchWorldOrigin;
    Math::Shape shape;
    Math::CreateShapeFromHeightmapImage(data, width, height, components, spacing, patchSize,
        [&shape](const Math::Vector3& vertex)
    {
        shape.vertexData_.push_back(vertex);
        ++shape.vertexCount_;
    },
        [&shape](int i1, int i2, int i3)
    {
        shape.AddTriangle(i1, i2, i3);
    },
        patchWorldSize, numPatches, numVertices, patchWorldOrigin);
    free(data);
    std::cout << "patchWorldSize: " << patchWorldSize << std::endl;
    std::cout << "numPatches: " << numPatches << std::endl;
    std::cout << "numVertices: " << numVertices << std::endl;
    std::cout << "patchWorldOrigin: " << patchWorldOrigin << std::endl;

    if (!IO::SaveShapeToOBJ(outputFile, shape))
    {
        std::cerr << "Failed to save to " << outputFile << std::endl;
        return 1;
    }
    std::cout << "Created " << outputFile << std::endl;

    return 0;
}
