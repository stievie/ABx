#include "stdafx.h"
#include "Terrain.h"

namespace Game {

Terrain::Terrain()
{
}


Terrain::~Terrain()
{
}

void Terrain::BuildShape()
{
    hullShape_ = std::make_unique<Math::ConvexHull>(vertices_);
}

}