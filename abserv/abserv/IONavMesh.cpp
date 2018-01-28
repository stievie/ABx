#include "stdafx.h"
#include "IONavMesh.h"
#include "DetourNavMesh.h"
#include "Profiler.h"

namespace IO {

static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;

struct NavMeshSetHeader
{
    int magic;
    int version;
    int numTiles;
    dtNavMeshParams params;
};

struct NavMeshTileHeader
{
    dtTileRef tileRef;
    int dataSize;
};

bool IONavMesh::Import(Game::NavigationMesh* asset, const std::string& fileName)
{
    AB_PROFILE;

#ifdef _WIN32
    FILE* fp;
    errno_t err = fopen_s(&fp, fileName.c_str(), "rb");
    if (err)
        return false;
#else
    FILE* fp = fopen(fileName.c_str(), "rb");
    if (!fp)
        return false;
#endif

    // Read header.
    NavMeshSetHeader header;
    size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
    if (readLen != 1)
    {
        fclose(fp);
        return false;
    }
    if (header.magic != NAVMESHSET_MAGIC)
    {
        fclose(fp);
        return false;
    }
    if (header.version != NAVMESHSET_VERSION)
    {
        fclose(fp);
        return false;
    }

    dtNavMesh* mesh = dtAllocNavMesh();
    if (!mesh)
    {
        fclose(fp);
        return false;
    }
    asset->SetNavMesh(mesh);

    dtStatus status = mesh->init(&header.params);
    if (dtStatusFailed(status))
    {
        fclose(fp);
        return false;
    }

    // Read tiles.
    for (int i = 0; i < header.numTiles; ++i)
    {
        NavMeshTileHeader tileHeader;
        readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
        if (readLen != 1)
        {
            fclose(fp);
            return false;
        }

        if (!tileHeader.tileRef || !tileHeader.dataSize)
            break;

        unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
        if (!data) break;
        memset(data, 0, tileHeader.dataSize);
        readLen = fread(data, tileHeader.dataSize, 1, fp);
        if (readLen != 1)
        {
            fclose(fp);
            return false;
        }

        mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
    }

    fclose(fp);
    return true;
}

}
