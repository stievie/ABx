#include "stdafx.h"
#include "CreateHeightMapAction.h"
#include "ObjWriter.h"
#include <fstream>
#include "MathUtils.h"

void CreateHeightMapAction::SaveObj()
{
    std::fstream f(file_ + ".obj", std::fstream::out);
    ObjWriter writer(f);
    writer.Comment(file_);
    writer.Object("heightmap");

    if (vertices_.size() < 3)
        return;

    writer.Comment(std::to_string(vertices_.size()) + " vertices");
    for (const auto& v : vertices_)
    {
        writer.Vertex(v.x, v.y, v.z);
    }
    writer.Comment(std::to_string(inidices_.size()) + " indices");
    for (size_t i = 0; i < inidices_.size(); )
    {
        writer.BeginFace();
        // Indices in OBJ are 1-based
        writer << inidices_[i] + 1 << inidices_[i + 1] + 1 << inidices_[i + 2] + 1;
        writer.EndFace();

        i += 3;
    }

    f.close();
}

void CreateHeightMapAction::CreateGeometry()
{
    vertices_.resize(width_ * height_);
    for (int x = 0; x < width_; x++)
    {
        for (int z = 0; z < height_; z++)
        {
            float y = GetRawHeight(x, z);
            float fx = (float)x - (float)width_ / 2.0f;
            float fz = (float)(height_ - z) - (float)height_ / 2.0f;
            vertices_[x * width_ + z] = {
                fx,
                y,
                fz
            };
        }
    }

    // Create index data
    for (int z = 0; z < height_ - 1; z++)
    {
        for (int x = 0; x < width_ - 1; x++)
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
            // First triangle
            // P1
            inidices_.push_back(x * width_ + z);
            // P2
            inidices_.push_back((x + 1) * width_ + z);
            // P3
            inidices_.push_back((x + 1) * width_ + z + 1);

            // Second triangle
            // P3
            inidices_.push_back((x + 1) * width_ + z + 1);
            // P2
            inidices_.push_back(x * width_ + z + 1);
            // P1
            inidices_.push_back(x * width_ + z);
        }
    }
}

float CreateHeightMapAction::GetRawHeight(int x, int z) const
{
    if (!data_)
        return 0.0f;

    int offset = (z * width_ + x) * components_;
    float r = (float)data_[offset] / 256.0f;
    float g = (float)data_[offset + 1] / 256.0f;
    float b = (float)data_[offset + 2] / 256.0f;
    return r + g + b;
}

void CreateHeightMapAction::Execute()
{
    data_ = stbi_load(file_.c_str(), &width_, &height_, &components_, 3);

    if (!data_)
    {
        std::cout << "Error loading file " << file_ << std::endl;
        return;
    }

    CreateGeometry();

    SaveObj();

}
