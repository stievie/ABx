#pragma once

#include <assimp/vector3.h>
#include <assimp/vector2.h>
#pragma warning(push)
#pragma warning(disable: 4244 4456)
#include <stb_image.h>
#pragma warning(pop)

typedef aiVector2t<int> PointI;

/// Creates Heightmap and Mesh from Image
class CreateHeightMapAction
{
private:
    std::vector<aiVector3D> vertices_;
    std::string file_;
    /// Vertex and height spacing.
    aiVector3D spacing_;
    std::vector<float> heightData_;
    float minHeight_;
    float maxHeight_;
    std::vector<unsigned short> indices_;
    std::vector<aiVector3D> normals_;
    float heightFactor_;
    int spacingX_;
    int spacingZ_;

    PointI numVertices_;
    int width_;
    int height_;
    int components_;
    stbi_uc* data_;
    void SaveObj();
    void SaveHeightMap();
    void CreateGeometry();
    float GetRawHeight(int x, int z) const;
    aiVector3D GetRawNormal(int x, int z) const;
public:
    CreateHeightMapAction(const std::string& file, float heightFactor = 1.0f, int spacingX = 1, int spacingZ = 1) :
        file_(file),
        data_(nullptr),
        width_(0),
        height_(0),
        heightFactor_(heightFactor),
        spacingX_(spacingX),
        spacingZ_(spacingZ)
    {}
    ~CreateHeightMapAction()
    {
        if (data_)
            free(data_);
    }
    void Execute();
};

