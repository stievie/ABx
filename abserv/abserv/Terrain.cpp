#include "stdafx.h"
#include "Terrain.h"
#include "TerrainPatch.h"

namespace Game {

Terrain::Terrain() :
    heightMap_(nullptr),
    patchSize_(DEFAULT_PATCH_SIZE)
{
}

Terrain::~Terrain()
{
}

}