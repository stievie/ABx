#pragma once

#include <string>
#pragma warning(push)
#pragma warning(disable: 4244 4456)
#include <stb_image.h>
#pragma warning(pop)

class MeshLoader
{
private:
    // Explicitly disabled copy constructor and copy assignment operator.
    MeshLoader(const MeshLoader&) = delete;
    MeshLoader& operator=(const MeshLoader&) = delete;

    void addVertex(float x, float y, float z, int& cap);
    void addTriangle(int a, int b, int c, int& cap);
    float GetHeight(int x, int z) const;

    std::string m_filename;
    float m_scale;
    float* m_verts;
    int* m_tris;
    float* m_normals;
    int m_vertCount;
    int m_triCount;

    int width_;
    int height_;
    int components_;
    stbi_uc* data_;
public:
    MeshLoader();
    ~MeshLoader();

    bool load(const std::string& fileName);
    bool loadHeightmap(const std::string& fileName, float scaleX, float scaleY, float scaleZ);

    const float* getVerts() const { return m_verts; }
    const float* getNormals() const { return m_normals; }
    const int* getTris() const { return m_tris; }
    int getVertCount() const { return m_vertCount; }
    int getTriCount() const { return m_triCount; }
    const std::string& getFileName() const { return m_filename; }
};

