#include "MeshLoader.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/scene.h>           // Output data structure
#include "MathUtils.h"
#include <fstream>

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

float MeshLoader::GetHeight(int x, int z, bool rightHand) const
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

bool MeshLoader::load(const std::string& fileName)
{
    m_vcap = 0;
    m_tcap = 0;
    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(fileName.c_str(),
        aiProcess_OptimizeMeshes
        | aiProcess_FindDegenerates
        | aiProcess_FindInvalidData

        | aiProcess_Triangulate

        // aiProcessPreset_TargetRealtime_Fast
//        aiProcess_CalcTangentSpace |
//        | aiProcess_GenNormals
//        aiProcess_JoinIdenticalVertices |
//        aiProcess_GenUVCoords |
//        aiProcess_SortByPType //|

        // aiProcess_ConvertToLeftHanded
//        aiProcess_MakeLeftHanded |
//        aiProcess_FlipUVs |
//        aiProcess_FlipWindingOrder //|

//        aiProcess_TransformUVCoords |
//        aiProcess_FixInfacingNormals
    );
    if (!scene)
        return false;

    // Add meshes
    unsigned int num_meshes = scene->mNumMeshes;
    vertices_.clear();
    indices_.clear();
    for (unsigned int nm = 0; nm < num_meshes; nm++)
    {
        const aiMesh* ai_mesh = scene->mMeshes[nm];

        // Add vertices
        for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++)
        {
            const aiVector3D* pos = &ai_mesh->mVertices[i];
            vertices_.push_back(*pos);
            addVertex(pos->x, pos->y, pos->z, m_vcap);
        }

        // Add indices
        for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++)
        {
            const aiFace* ai_face = &ai_mesh->mFaces[i];
            // We triangulate the faces so it must be always 3 vertices per face.
            if (ai_face->mNumIndices == 3)
            {
                indices_.push_back(ai_face->mIndices[0]);
                indices_.push_back(ai_face->mIndices[1]);
                indices_.push_back(ai_face->mIndices[2]);
                addTriangle(ai_face->mIndices[0], ai_face->mIndices[1], ai_face->mIndices[2], m_tcap);
            }
        }

    }

    CalculateNormals();
    m_filename = fileName;
    return true;
}

bool MeshLoader::loadHeightmap(const std::string& fileName, float scaleX, float scaleY, float scaleZ)
{
    m_vcap = 0;
    m_tcap = 0;
    data_ = stbi_load(fileName.c_str(), &width_, &height_, &components_, 0);

    if (!data_)
    {
        return false;
    }

    vertices_.resize(width_ * height_);
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            float fy = GetHeight(x, y);
            float fx = (float)x - ((float)width_ * 0.5f);
            float fz = (float)y -((float)height_ * 0.5f);
            vertices_[y * width_ + x] = {
                fx * scaleX, fy * scaleY, fz * scaleZ
            };
            addVertex(fx * scaleX, fy * scaleY, fz * scaleZ, m_vcap);
        }
    }

    // Create index data
    for (int y = 0; y < height_ - 1; ++y)
    {
        for (int x = 0; x < width_ - 1; ++x)
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
                int i1 = (y + 1) * width_ + x;
                int i2 = y * width_ + x;
                int i3 = (y * width_) + x + 1;
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
                int i1 = y * width_ + x + 1;
                int i2 = (y + 1) * width_ + (x + 1);
                int i3 = (y + 1) * width_ + x;
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
