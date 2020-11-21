#include "MeshLoader.h"
#include <fstream>
#include <cstring>
#include <iostream>
#include <absmath/HeightMapTools.h>

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

void MeshLoader::addVertex(float x, float y, float z)
{
    addVertex(x, y, z, m_vcap);
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

void MeshLoader::addTriangle(int a, int b, int c)
{
    addTriangle(a, b, c, m_tcap);
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
    data_ = stbi_load(fileName.c_str(), &width_, &height_, &components_, 0);

    if (!data_)
    {
        return false;
    }

    if ((width_ - 1) % patchSize != 0 || (height_ - 1) % patchSize != 0)
    {
        std::cout << "WARNING: Image size - 1 (" << width_ << "x" << height_<< ") should be a multiple of patch size (" << patchSize << ")" << std::endl;
    }

    Math::Vector2 patchWorldSize;
    Math::IntVector2 numPatches;
    Math::CreateShapeFromHeightmapImage((const unsigned char*)data_, width_, height_, components_, { scaleX , scaleY, scaleZ },
        patchSize,
        [this](const Math::Vector3& vertex)
        {
            addVertex(vertex.x_, vertex.y_, vertex.z_, m_vcap);
        },
        [this](int i1, int i2, int i3)
        {
            addTriangle(i1, i2, i3, m_tcap);
        },
        patchWorldSize, numPatches, numVertices_, patchWorldOrigin_
    );

    free(data_);
    return true;
}
