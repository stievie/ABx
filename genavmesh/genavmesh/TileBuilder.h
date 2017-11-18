#pragma once

#include "Builder.h"
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourDebugDraw.h"

class TileBuilder : public Builder
{
private:
    rcConfig m_cfg;
    rcHeightfield* m_solid;
    rcCompactHeightfield* m_chf;
    rcContourSet* m_cset;
    rcPolyMesh* m_pmesh;
    rcPolyMeshDetail* m_dmesh;
    float m_lastBuiltTileBmin[3];
    float m_lastBuiltTileBmax[3];
    float m_totalBuildTimeMs;
    float m_tileMemUsage;
    float m_tileBuildTime;
    int m_tileTriCount;
    int m_partitionType;
    void buildAllTiles(InputGeom* geom, const BuildSettings& settings);
    unsigned char* buildTileMesh(InputGeom* geom, const BuildSettings& settings,
        const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize);
    void cleanup();
public:
    TileBuilder(BuildContext* ctx) :
        Builder(ctx)
    {}
    virtual ~TileBuilder() {
        cleanup();
    }
    bool Build(InputGeom* geom, const BuildSettings& settings) override;
    bool Save(const char* path, const dtNavMesh* mesh) override;
};

