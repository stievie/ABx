/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include <sa/PragmaWarning.h>
#include "CreateModelAction.h"
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_GCC("-Wdeprecated-copy")
PRAGMA_WARNING_DISABLE_GCC("-Waddress-of-packed-member")
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/scene.h>
PRAGMA_WARNING_POP
#include <abscommon/StringUtils.h>

void CreateModelAction::Save()
{
    std::string fileName = Utils::ChangeFileExt(file_, ".mdl");
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
        " BB " << boundingBox_ <<
        std::endl;
    Save();
}
