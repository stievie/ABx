// genavmesh.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include "InputGeom.h"
#include "BuildContext.h"

void ShowUsage()
{
    std::cout << "Usage: genavmesh [options] files" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        ShowUsage();
        return 1;
    }
    BuildContext ctx;
    BuildSettings settings;
    // Rasterization
    settings.cellSize = 0.3f;
    settings.cellHeight = 0.2f;
    // Agent
    settings.agentHeight = 2.0f;
    settings.agentRadius = 0.6f;
    settings.agentMaxClimb = 0.9f;
    settings.agentMaxSlope = 45.0f;
    // Region
    settings.regionMinSize = 8.0f;
    settings.regionMergeSize = 20.0f;
    // Partitioning
    settings.partitionType = 1;
    // Filtering
    // Polygonization
    settings.edgeMaxLen = 12.0f;
    settings.edgeMaxError = 1.3f;
    settings.vertsPerPoly = 6;
    // Detail Mesh
    settings.detailSampleDist = 6.0f;
    settings.detailSampleMaxError = 1.0f;


    for (int i = 1; i < argc; i++)
    {
        InputGeom geom;
        std::string fn(argv[i]);
        std::cout << "Processing file " << fn << std::endl;
        if (!geom.load(&ctx, fn))
        {

            std::cout << "Error loading file " << fn << std::endl;
            continue;
        }
        rcVcopy(settings.navMeshBMin, geom.getNavMeshBoundsMin());
        rcVcopy(settings.navMeshBMax, geom.getNavMeshBoundsMax());
        if (!geom.saveGeomSet(&settings))
        {
            std::cout << "Error generating geometry set" << std::endl;
            continue;
        }
    }
    return 0;
}

