#include "stdafx.h"
#include "IOTerrain.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/scene.h>     // Post processing flags
#include <fstream>

namespace IO {

bool IOTerrain::Import(Game::Terrain* asset, const std::string& name)
{
    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(name,
        aiProcess_OptimizeMeshes |
        aiProcess_FindDegenerates |
        aiProcess_FindInvalidData |

        // aiProcessPreset_TargetRealtime_Fast
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_GenUVCoords |
        aiProcess_SortByPType |

        // aiProcess_ConvertToLeftHanded
        aiProcess_MakeLeftHanded |
        aiProcess_FlipUVs |
        aiProcess_FlipWindingOrder |

        aiProcess_TransformUVCoords |
        aiProcess_FixInfacingNormals
    );
    if (!scene)
        return false;

//    std::ofstream os(name + ".bin", std::ios::binary);

    // Add meshes
    unsigned int num_meshes = scene->mNumMeshes;
    for (unsigned int nm = 0; nm < num_meshes; nm++)
    {
        const aiMesh* ai_mesh = scene->mMeshes[nm];

        // Add vertices
        if (ai_mesh->HasPositions())
        {
            for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++)
            {
                aiVector3D* pos = &ai_mesh->mVertices[i];
                asset->vertices_.push_back({ pos->x, pos->y, pos->z });
//                os.write(reinterpret_cast<char*>(pos), sizeof(aiVector3D));
            }
        }
    }
    asset->BuildShape();
//    os.close();

    return true;
}

}

#pragma comment(lib, "assimp-vc140-mt.lib")
#ifdef _DEBUG
#pragma comment(lib, "zlibstaticd.lib")
#else
#pragma comment(lib, "zlibstatic.lib")
#endif
