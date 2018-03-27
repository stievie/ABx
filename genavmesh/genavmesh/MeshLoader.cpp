#include "MeshLoader.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/scene.h>           // Output data structure
#include "MathUtils.h"

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

float MeshLoader::GetHeight(int x, int z) const
{
    if (!data_)
        return 0.0f;

    if (x >= width_)
        x = width_ - 1;
    if (z >= height_)
        z = height_ - 1;
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

aiVector3D MeshLoader::GetRawNormal(int x, int z) const
{
    float baseHeight = GetHeight(x, z);
    float nSlope = GetHeight(x, z - 1) - baseHeight;
    float neSlope = GetHeight(x + 1, z - 1) - baseHeight;
    float eSlope = GetHeight(x + 1, z) - baseHeight;
    float seSlope = GetHeight(x + 1, z + 1) - baseHeight;
    float sSlope = GetHeight(x, z + 1) - baseHeight;
    float swSlope = GetHeight(x - 1, z + 1) - baseHeight;
    float wSlope = GetHeight(x - 1, z) - baseHeight;
    float nwSlope = GetHeight(x - 1, z - 1) - baseHeight;
    float up = 1.0f;

    aiVector3D result = Vector3Add(aiVector3D(0.0f, up, nSlope), aiVector3D(-neSlope, up, neSlope));
    result = Vector3Add(result, aiVector3D(-eSlope, up, 0.0f));
    result = Vector3Add(result, aiVector3D(-seSlope, up, -seSlope));
    result = Vector3Add(result, aiVector3D(0.0f, up, -sSlope));
    result = Vector3Add(result, aiVector3D(swSlope, up, -swSlope));
    result = Vector3Add(result, aiVector3D(wSlope, up, 0.0f));
    result = Vector3Add(result, aiVector3D(nwSlope, up, nwSlope));
    // Normalize
    float length = sqrt(result.x * result.x + result.y * result.y + result.z * result.z);
    if (length > 0.0f)
    {
        result.x /= length;
        result.y /= length;
        result.z /= length;
    }
    //    result.Normalize
    return result;
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

/*    normals_.clear();
    for (size_t i = 0; i < indices_.size(); i += 3)
    {
        const aiVector3D v0 = vertices_[indices_[i]];
        const aiVector3D v1 = vertices_[indices_[i + 1]];
        const aiVector3D v2 = vertices_[indices_[i + 2]];
        aiVector3D e0 = Vector3Sub(v0, v1);
        aiVector3D e1 = Vector3Sub(v0, v2);
        aiVector3D n = CrossProduct(e0, e1);
        float d = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
        if (d > 0)
        {
            d = 1.0f / d;
            n.x *= d;
            n.y *= d;
            n.z *= d;
        }
        normals_.push_back(n);
    }
  */
}

bool MeshLoader::load(const std::string& fileName)
{
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

    int vcap = 0;
    int tcap = 0;
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
            addVertex(pos->x, pos->y, pos->z, vcap);
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
                addTriangle(ai_face->mIndices[0], ai_face->mIndices[1], ai_face->mIndices[2], tcap);
            }
        }

    }

    CalculateNormals();
    m_filename = fileName;
    return true;
}

bool MeshLoader::loadHeightmap(const std::string& fileName, float scaleX, float scaleY, float scaleZ)
{
    data_ = stbi_load(fileName.c_str(), &width_, &height_, &components_, 0);

    if (!data_)
    {
        return false;
    }

    int vcap = 0;
    int tcap = 0;
    vertices_.resize(width_ * height_);
    for (int x = 0; x < width_; x++)
    {
        for (int z = 0; z < height_; z++)
        {
            float fy = GetHeight(x, z);
            float fx = (float)x - (float)width_ / 2.0f;
            float fz = (float)z - (float)height_ / 2.0f;
            vertices_[z * width_ + x] = {
                fx * scaleX, fy * scaleY, fz * scaleZ
            };
            normals_.push_back(GetRawNormal(x, z));
            addVertex(fx * scaleX, fy * scaleY, fz * scaleZ, vcap);
        }
    }

    // Create index data
    for (int x = 0; x < width_ - 1; x++)
    {
        for (int z = 0; z < height_ - 1; z++)
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
                int i1 = z * width_ + x;
                int i2 = (z * width_) + x + 1;
                int i3 = (z + 1) * width_ + (x + 1);
                // P1
                indices_.push_back(i1);
                // P2
                indices_.push_back(i2);
                // P3
                indices_.push_back(i3);
                addTriangle(i1, i2, i3, tcap);
            }

            {
                // Second triangle
                int i3 = (z + 1) * width_ + (x + 1);
                int i2 = (z + 1) * width_ + x;
                int i1 = z * width_ + x;
                // P3
                indices_.push_back(i3);
                // P2
                indices_.push_back(i2);
                // P1
                indices_.push_back(i1);
                addTriangle(i3, i2, i1, tcap);
            }
        }
    }

    CalculateNormals();

    free(data_);
    return true;
}
