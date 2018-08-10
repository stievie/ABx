#include "stdafx.h"
#include "CreateModelAction.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/scene.h>

void CreateModelAction::Save()
{
    std::string fileName = file_ + ".model";
    std::ofstream output(fileName, std::ios::binary);
    output.write((char*)"MODL", 4);

    output.write((char*)&boundingBox_.min_.x_, sizeof(float));
    output.write((char*)&boundingBox_.min_.y_, sizeof(float));
    output.write((char*)&boundingBox_.min_.z_, sizeof(float));
    output.write((char*)&boundingBox_.max_.x_, sizeof(float));
    output.write((char*)&boundingBox_.max_.y_, sizeof(float));
    output.write((char*)&boundingBox_.max_.z_, sizeof(float));

    output.write((char*)&vertexCount_, sizeof(vertexCount_));
    for (const auto& v : vertexData_)
    {
        output.write((char*)&v.x_, sizeof(float));
        output.write((char*)&v.y_, sizeof(float));
        output.write((char*)&v.z_, sizeof(float));
    }
    output.write((char*)&indexCount_, sizeof(indexCount_));
    for (const auto& i : indexData_)
    {
        output.write((char*)&i, sizeof(unsigned));
    }
    output.close();
    std::cout << "Created " << fileName << std::endl;
}

void CreateModelAction::Execute()
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
                vertexData_.push_back({
                    ai_mesh->mVertices[i].x,
                    ai_mesh->mVertices[i].y,
                    ai_mesh->mVertices[i].z,
                });
                vertexCount_++;
            }
        }

        // Add indices
        for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++)
        {
            const aiFace* ai_face = &ai_mesh->mFaces[i];
            // We triangulate the faces so it must be always 3 vertices per face.
            if (ai_face->mNumIndices == 3)
            {
                indexData_.push_back((unsigned short)ai_face->mIndices[0]);
                indexData_.push_back((unsigned short)ai_face->mIndices[1]);
                indexData_.push_back((unsigned short)ai_face->mIndices[2]);
                indexCount_ += 3;
            }
            else
                // or we just don't draw it.
                std::cout << "Invalid/unsupported number of indices, expected 3 but got " <<
                    static_cast<int>(ai_face->mNumIndices) << std::endl;
        }

    }
    boundingBox_.Reset();
    boundingBox_.Merge(vertexData_.data(), vertexCount_);

    std::cout << "Mesh has " << vertexData_.size() << " vertices and " << indexData_.size() << " indices, " <<
        " BB " << boundingBox_.ToString() <<
        std::endl;
    Save();
}
