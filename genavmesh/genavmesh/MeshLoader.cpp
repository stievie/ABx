#include "MeshLoader.h"
#include "MathUtils.h"
#include <fstream>
#include <absmath/Point.h>
#include <cstring>

MeshLoader::MeshLoader() :
    m_scale(1.0f),
    m_verts(0),
    m_tris(0),
    m_normals(0),
    m_vertCount(0),
    m_triCount(0)
{
}

MeshLoader::~MeshLoader()
{
    delete[] m_verts;
    delete[] m_normals;
    delete[] m_tris;
}

void MeshLoader::addVertex(float x, float y, float z, int& cap)
{
    if (m_vertCount + 1 > cap)
    {
        cap = !cap ? 8 : cap * 2;
        float* nv = new float[cap * 3];
        if (m_vertCount)
            memcpy(nv, m_verts, m_vertCount * 3 * sizeof(float));
        delete[] m_verts;
        m_verts = nv;
    }
    float* dst = &m_verts[m_vertCount * 3];
    *dst++ = x*m_scale;
    *dst++ = y*m_scale;
    *dst++ = z*m_scale;
    m_vertCount++;
}

void MeshLoader::addTriangle(int a, int b, int c, int& cap)
{
    if (m_triCount + 1 > cap)
    {
        cap = !cap ? 8 : cap * 2;
        int* nv = new int[cap * 3];
        if (m_triCount)
            memcpy(nv, m_tris, m_triCount * 3 * sizeof(int));
        delete[] m_tris;
        m_tris = nv;
    }
    int* dst = &m_tris[m_triCount * 3];
    *dst++ = a;
    *dst++ = b;
    *dst++ = c;
    m_triCount++;
}

void MeshLoader::CalculateNormals()
{
    // Calculate normals.
    m_normals = new float[m_triCount * 3];
    for (int i = 0; i < m_triCount * 3; i += 3)
    {
        const float* v0 = &m_verts[m_tris[i] * 3];
        const float* v1 = &m_verts[m_tris[i + 1] * 3];
        const float* v2 = &m_verts[m_tris[i + 2] * 3];
        float e0[3], e1[3];
        for (int j = 0; j < 3; ++j)
        {
            e0[j] = v1[j] - v0[j];
            e1[j] = v2[j] - v0[j];
        }
        float* n = &m_normals[i];
        n[0] = e0[1] * e1[2] - e0[2] * e1[1];
        n[1] = e0[2] * e1[0] - e0[0] * e1[2];
        n[2] = e0[0] * e1[1] - e0[1] * e1[0];
        float d = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        if (d > 0)
        {
            d = 1.0f / d;
            n[0] *= d;
            n[1] *= d;
            n[2] *= d;
        }
    }
}

bool MeshLoader::loadHeightmap(const std::string& fileName, float scaleX, float scaleY, float scaleZ, int patchSize)
{
    m_vcap = 0;
    m_tcap = 0;
    data_ = stbi_load(fileName.c_str(), &width_, &height_, &components_, 0);

    if (!data_)
    {
        return false;
    }

    Math::Point<int> numPatches = { (width_ - 1) / patchSize, (height_ - 1) / patchSize };
    Math::Point<int> numVertices = { numPatches.x_ * patchSize + 1, numPatches.y_ * patchSize + 1 };

    auto getHeight = [&](int x, int z, bool rightHand = false) -> float
    {
        if (!data_)
            return 0.0f;

        // From bottom to top
        int offset;
        if (rightHand)
            offset = (((height_ - 1) - z) * width_ + ((width_ - 1) - x)) * components_;
        else
            offset = (((height_ - 1) - z) * width_ + x) * components_;

        if (components_ == 1)
            return (float)data_[offset];

        // If more than 1 component, use the green channel for more accuracy
        return (float)data_[offset] +
            (float)data_[offset + 1] / 256.0f;
    };

    vertices_.resize(numVertices.x_ * numVertices.y_);
    for (int y = 0; y < numVertices.y_; ++y)
    {
        for (int x = 0; x < numVertices.x_; ++x)
        {
            float fy = getHeight(x, y);
            float fx = ((float)x - ((float)numVertices.x_ * 0.5f));
            float fz = (float)y -((float)numVertices.y_ * 0.5f);

            vertices_[y * numVertices.x_ + x] = {
                fx * scaleX, fy * scaleY, fz * scaleZ
            };
            addVertex(fx * scaleX, fy * scaleY, fz * scaleZ, m_vcap);
        }
    }

    // Create index data
    for (int y = 0; y < numVertices.y_ - 1; ++y)
    {
        for (int x = 0; x < numVertices.x_ - 1; ++x)
        {
            /*
                x+1,y
        x,y +----+----+
            | 1 /|(3)/|
            |  / |  / |
            | /  | /  |
            |/ 2 |/(4)|
      x,y+1 +----+----+
              x+1,y+1
            */
            {
                // First triangle
                int i1 = (y + 1) * numVertices.x_ + x;
                int i2 = y * numVertices.x_ + x;
                int i3 = (y * numVertices.x_) + x + 1;
                // P1
                indices_.push_back(i3);
                // P2
                indices_.push_back(i2);
                // P3
                indices_.push_back(i1);
                addTriangle(i3, i2, i1, m_tcap);
            }

            {
                // Second triangle
                int i1 = y * numVertices.x_ + x + 1;
                int i2 = (y + 1) * numVertices.x_ + (x + 1);
                int i3 = (y + 1) * numVertices.x_ + x;
                // P3
                indices_.push_back(i3);
                // P2
                indices_.push_back(i2);
                // P1
                indices_.push_back(i1);
                addTriangle(i3, i2, i1, m_tcap);
            }
        }
    }

    CalculateNormals();

    free(data_);
    return true;
}
