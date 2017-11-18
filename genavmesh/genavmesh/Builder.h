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

/// These are just sample areas to use consistent values across the samples.
/// The use should specify these base on his needs.
enum SamplePolyAreas
{
    SAMPLE_POLYAREA_GROUND,
    SAMPLE_POLYAREA_WATER,
    SAMPLE_POLYAREA_ROAD,
    SAMPLE_POLYAREA_DOOR,
    SAMPLE_POLYAREA_GRASS,
    SAMPLE_POLYAREA_JUMP,
};
enum SamplePolyFlags
{
    SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
    SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
    SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
    SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
    SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
    SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
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

