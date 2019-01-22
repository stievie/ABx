#pragma once

#pragma warning(push)
#pragma warning(disable: 4244 4456)
#include <stb_image.h>
#pragma warning(pop)
#include "Vector3.h"
#include "Point.h"

/// Creates Heightmap and Mesh from Image
class CreateHeightMapAction
{
private:
    std::vector<Math::Vector3> vertices_;
    std::string file_;
    /// Vertex and height spacing.
    Math::Vector3 spacing_;
    std::vector<float> heightData_;
    float minHeight_;
    float maxHeight_;
    std::vector<unsigned short> indices_;
    std::vector<Math::Vector3> normals_;

    int width_;
    int height_;
    int components_;
    stbi_uc* data_;
    void SaveObj();
    void SaveHeightMap();
    void CreateGeometry();
    float GetRawHeight(int x, int z) const;
    Math::Vector3 GetRawNormal(int x, int z) const;
public:
    CreateHeightMapAction(const std::string& file) :
        file_(file),
        data_(nullptr),
        width_(0),
        height_(0),
        spacing_(Math::Vector3(1.0f, 0.2f, 1.0f))
    {}
    ~CreateHeightMapAction()
    {
        if (data_)
            free(data_);
    }
    void Execute();
};

