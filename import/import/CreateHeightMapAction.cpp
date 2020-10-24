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

#include "stdafx.h"
#include "CreateHeightMapAction.h"
#include "ObjWriter.h"
#include <fstream>
#include <absmath/MathUtils.h>
#include <limits>
#include <abscommon/StringUtils.h>

void CreateHeightMapAction::SaveObj()
{
    std::string fileName = Utils::ChangeFileExt(file_, ".obj");
    std::fstream f(fileName, std::fstream::out);
    ObjWriter writer(f, false);
    writer.Comment(file_);
    writer.Object("heightmap");

    if (vertices_.size() < 3)
        return;

    writer.Comment(std::to_string(vertices_.size()) + " vertices");
    for (const auto& v : vertices_)
    {
        const Math::Vector3 vec = v * spacing_;
        writer.Vertex(vec.x_, vec.y_, vec.z_);
    }
    writer.Comment(std::to_string(indices_.size()) + " normals");
    for (const auto& n : normals_)
    {
        writer.Normal(n.x_, n.y_, n.z_);
    }
    writer.Comment(std::to_string(indices_.size()) + " indices");
    for (size_t i = 0; i < indices_.size(); )
    {
        writer.BeginFace();
        // Indices in OBJ are 1-based
        writer << indices_[i] + 1 << indices_[i + 1] + 1 << indices_[i + 2] + 1;
        writer.EndFace();

        i += 3;
    }

    f.close();

    std::cout << "Created " << fileName << std::endl;
}

void CreateHeightMapAction::SaveHeightMap()
{
    std::string fileName = Utils::ChangeFileExt(file_, ".hm");
    std::fstream output(fileName, std::ios::binary | std::fstream::out);
    output.write((char*)"HM\0\0", 4);
    // Height Data
    output.write((char*)&width_, sizeof(width_));
    output.write((char*)&height_, sizeof(height_));

    output.write((char*)&minHeight_, sizeof(minHeight_));
    output.write((char*)&maxHeight_, sizeof(maxHeight_));
    unsigned c = (unsigned)heightData_.size();
    output.write((char*)&c, sizeof(c));
    output.write((char*)heightData_.data(), c * sizeof(float));

    // Shape data
    unsigned vertexCount = (unsigned)vertices_.size();
    output.write((char*)&vertexCount, sizeof(vertexCount));
    for (const auto& v : vertices_)
    {
        output.write((char*)&v.x_, sizeof(float));
        output.write((char*)&v.y_, sizeof(float));
        output.write((char*)&v.z_, sizeof(float));
    }
    unsigned indexCount = (unsigned)indices_.size();
    output.write((char*)&indexCount, sizeof(indexCount));
    for (const auto& i : indices_)
    {
        output.write((char*)&i, sizeof(unsigned));
    }

    output.close();
    std::cout << "Created " << fileName << std::endl;
}

void CreateHeightMapAction::CreateGeometry()
{
    minHeight_ = std::numeric_limits<float>::max();
    maxHeight_ = std::numeric_limits<float>::lowest();
    vertices_.resize(width_ * height_);
    normals_.resize(width_ * height_);
    heightData_.resize(width_ * height_);
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            float fy = GetRawHeight(x, y);
            heightData_[y * width_ + x] = fy;
            if (minHeight_ > fy)
                minHeight_ = fy;
            if (maxHeight_ < fy)
                maxHeight_ = fy;
            float fx = (float)x - (float)width_ * 0.5f;
            float fz = (float)y - (float)height_ * 0.5f;
            vertices_[y * width_ + x] = {
                fx,
                fy,
                fz
            };

            normals_.push_back(GetRawNormal(x, y));
        }
    }

    // Create index data in clockwise order
    for (int y = 0; y < height_ - 1; ++y)
    {
        for (int x = 0; x < width_ - 1; ++x)
        {
            /*
            Normal edge:
            +----+----+
            |\ 1 |\   |
            | \  | \  |
            |  \ |  \ |
            | 2 \|   \|
            +----+----+
            */
            {
                // First triangle
                int i1 = (y + 1) * width_ + x;
                int i2 = y * width_ + x;
                int i3 = (y * width_) + x + 1;
                // P1
                indices_.push_back(static_cast<unsigned short>(i3));
                // P2
                indices_.push_back(static_cast<unsigned short>(i2));
                // P3
                indices_.push_back(static_cast<unsigned short>(i1));
            }

            {
                // Second triangle
                int i1 = y * width_ + x + 1;
                int i2 = (y + 1) * width_ + (x + 1);
                int i3 = (y + 1) * width_ + x;
                // P3
                indices_.push_back(static_cast<unsigned short>(i3));
                // P2
                indices_.push_back(static_cast<unsigned short>(i2));
                // P1
                indices_.push_back(static_cast<unsigned short>(i1));
            }
        }
    }
}

float CreateHeightMapAction::GetRawHeight(int x, int z) const
{
    if (!data_)
        return 0.0f;

    // From bottom to top
    int offset = ((height_ - z) * width_ + x) * components_;
    if (components_ == 1)
    {
        return (float)data_[offset];
    }
    // If more than 1 component, use the green channel for more accuracy
    return (float)data_[offset] +
        (float)data_[offset + 1] / 256.0f;
}

Math::Vector3 CreateHeightMapAction::GetRawNormal(int x, int z) const
{
    float baseHeight = GetRawHeight(x, z);
    float nSlope = GetRawHeight(x, z - 1) - baseHeight;
    float neSlope = GetRawHeight(x + 1, z - 1) - baseHeight;
    float eSlope = GetRawHeight(x + 1, z) - baseHeight;
    float seSlope = GetRawHeight(x + 1, z + 1) - baseHeight;
    float sSlope = GetRawHeight(x, z + 1) - baseHeight;
    float swSlope = GetRawHeight(x - 1, z + 1) - baseHeight;
    float wSlope = GetRawHeight(x - 1, z) - baseHeight;
    float nwSlope = GetRawHeight(x - 1, z - 1) - baseHeight;
    float up = 0.5f * (spacing_.x_ + spacing_.z_);

    using namespace Math;
    return (Vector3(0.0f, up, nSlope) +
        Vector3(-neSlope, up, neSlope) +
        Vector3(-eSlope, up, 0.0f) +
        Vector3(-seSlope, up, -seSlope) +
        Vector3(0.0f, up, -sSlope) +
        Vector3(swSlope, up, -swSlope) +
        Vector3(wSlope, up, 0.0f) +
        Vector3(nwSlope, up, nwSlope)).Normal();
}

void CreateHeightMapAction::Execute()
{
    data_ = stbi_load(file_.c_str(), &width_, &height_, &components_, 0);

    if (!data_)
    {
        std::cout << "Error loading file " << file_ << std::endl;
        return;
    }

    CreateGeometry();

    SaveHeightMap();
    SaveObj();

}
