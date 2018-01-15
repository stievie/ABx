#include "stdafx.h"
#include "CreateHullAction.h"
#include "Hull.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/scene.h>

void CreateHullAction::BuildHull(const std::vector<aiVector3D>& vertices)
{
    if (vertices.size())
    {
        // Build the convex hull from the raw geometry
        StanHull::HullDesc desc;
        desc.SetHullFlag(StanHull::QF_TRIANGLES);
        desc.mVcount = (unsigned)vertices.size();
        desc.mVertices = &vertices[0].x;
        desc.mVertexStride = 3 * sizeof(float);
        desc.mSkinWidth = 0.0f;

        StanHull::HullLibrary lib;
        StanHull::HullResult result;
        lib.CreateConvexHull(desc, result);

        vertexCount_ = result.mNumOutputVertices;
        vertexData_.resize(vertexCount_);

        indexCount_ = result.mNumIndices;
        indexData_.resize(indexCount_);

        // Copy vertex data & index data
        memcpy(vertexData_.data(), result.mOutputVertices, vertexCount_ * sizeof(aiVector3D));
        memcpy(indexData_.data(), result.mIndices, indexCount_ * sizeof(unsigned));

        lib.ReleaseResult(result);

    }
    else
    {
        vertexCount_ = 0;
        indexCount_ = 0;
    }
}

void CreateHullAction::Save()
{
    std::ofstream output(file_ + ".hull", std::ios::binary);
    output.write((char*)"HULL", 4);
    output.write((char*)&vertexCount_, sizeof(vertexCount_));
    for (const auto& v : vertexData_)
    {
        output.write((char*)&v.x, sizeof(float));
        output.write((char*)&v.y, sizeof(float));
        output.write((char*)&v.z, sizeof(float));
    }
    output.write((char*)&indexCount_, sizeof(indexCount_));
    for (const auto& i : indexData_)
    {
        output.write((char*)&i, sizeof(unsigned));
    }
    output.close();
}

void CreateHullAction::Execute()
{
    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(file_.c_str(),
        aiProcess_OptimizeMeshes |
        aiProcess_FindDegenerates |
        aiProcess_FindInvalidData |

        // aiProcessPreset_TargetRealtime_Fast
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_SortByPType |

        // aiProcess_ConvertToLeftHanded
        aiProcess_MakeLeftHanded |
        aiProcess_FlipUVs |
        aiProcess_FlipWindingOrder |

        aiProcess_FixInfacingNormals
    );
    if (!scene)
        return;

    std::vector<aiVector3D> vertices;
    // Add meshes
    unsigned int num_meshes = scene->mNumMeshes;
    for (unsigned int nm = 0; nm < num_meshes; nm++)
    {
        const aiMesh* ai_mesh = scene->mMeshes[nm];

        // Add vertices
        for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++)
        {
            if (ai_mesh->HasPositions())
            {
                vertices.push_back(ai_mesh->mVertices[i]);
            }
        }
    }
    std::cout << "Mesh has " << vertices.size() << " vertices" << std::endl;
    BuildHull(vertices);
    std::cout << "Hull has " << vertexCount_ << " vertices" << " and " << indexCount_ << " indices" << std::endl;
    Save();
}
