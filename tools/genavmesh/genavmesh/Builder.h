#pragma once

#include "InputGeom.h"
#include "DetourNavMesh.h"
#include "BuildContext.h"
#include "DetourNavMeshQuery.h"

enum SamplePartitionType
{
    SAMPLE_PARTITION_WATERSHED,
    SAMPLE_PARTITION_MONOTONE,
    SAMPLE_PARTITION_LAYERS,
};

class Builder
{
protected:
    class dtNavMesh* m_navMesh;
    class dtNavMeshQuery* m_navQuery;
    BuildContext* m_ctx;
public:
    Builder(BuildContext* ctx) :
        m_ctx(ctx),
        m_navMesh(nullptr)
    {
        m_navQuery = dtAllocNavMeshQuery();
    }
    virtual ~Builder();

    virtual bool Build(InputGeom* geom, const BuildSettings& settings) = 0;
    virtual bool Save(const char* path, const dtNavMesh* mesh) = 0;

    dtNavMesh* GetNavMesh() const { return m_navMesh; }

    bool m_filterLowHangingObstacles;
    bool m_filterLedgeSpans;
    bool m_filterWalkableLowHeightSpans;
};

