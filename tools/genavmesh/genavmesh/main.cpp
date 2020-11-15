#include <iostream>
#include "InputGeom.h"
#include "BuildContext.h"
#include "TileBuilder.h"
#include <string>
#include <vector>
#include "ConfigFile.h"

static bool verbose = false;
static bool noObj = true;

static void ShowUsage()
{
    std::cout << "Generate Detour navmesh from height map" << std::endl << std::endl;
    std::cout << "Usage: genavmesh [-<options>] files" << std::endl;
    std::cout << "options (format is -<name>:<value>):" << std::endl;
    std::cout << "  cs, cell-size: cell size, default 0.3" << std::endl;
    std::cout << "  ch, cell-height: cell height, default 0.2" << std::endl;
    std::cout << "  ah, agent-height: agent height, default 2.0" << std::endl;
    std::cout << "  ar, agent-radius: agent radius, default 0.6" << std::endl;
    std::cout << "  amc: agent max climb, default 0.9" << std::endl;
    std::cout << "  ams: agent max slope, default 45.0" << std::endl;
    std::cout << "  rms: region min size, default 8.0" << std::endl;
    std::cout << "  rmes: region merge size, default 20.0" << std::endl;
    std::cout << "  pt: partition type, default 1 (Watershed)" << std::endl;
    std::cout << "  eml: edge max len, default 12.0" << std::endl;
    std::cout << "  eme: edge max error, default 1.3" << std::endl;
    std::cout << "  vpp: verts per poly, default 6" << std::endl;
    std::cout << "  dsd: detail sample dist, default 6.0" << std::endl;
    std::cout << "  dsmm: detail sample max error, default 1.0" << std::endl;
    std::cout << "  ts: tile size, default 32" << std::endl;
    std::cout << "  hmsx: heightmap scaling X, default 1.0" << std::endl;
    std::cout << "  hmsy: heightmap scaling Y, default 0.25" << std::endl;
    std::cout << "  hmsz: heightmap scaling Z, default 1.0" << std::endl;
    std::cout << "  verbose: Log to std out" << std::endl;
    std::cout << "  createobj: Create .obj file heightfield" << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  genavmesh -cs:0.4 sourcemesh.png" << std::endl;
}

static std::vector<std::string> split(const std::string& s, char seperator)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while ((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring(s.substr(prev_pos, pos - prev_pos));
        output.push_back(substring);
        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

    return output;
}

static void ParseArg(const std::string& arg, BuildSettings& settings)
{
    if (arg.compare("verbose") == 0)
    {
        verbose = true;
        return;
    }
    else if (arg.compare("createobj") == 0)
    {
        noObj = false;
        return;
    }

    std::vector<std::string> parts = split(arg, ':');
    if (parts.size() != 2)
        return;
    const std::string& name = parts[0];
    const std::string& value = parts[1];
    // Rasterization
    if (name.compare("cs") == 0 || name.compare("cell-size") == 0)
    {
        settings.cellSize = (float)std::atof(value.c_str());
    }
    else if (name.compare("ch") == 0 || name.compare("cell-height") == 0)
    {
        settings.cellHeight = (float)std::atof(value.c_str());
    }
    // Agent
    else if (name.compare("ah") == 0 || name.compare("agent-height") == 0)
    {
        settings.agentHeight = (float)std::atof(value.c_str());
    }
    else if (name.compare("ar") == 0 || name.compare("agent-radius") == 0)
    {
        settings.agentRadius = (float)std::atof(value.c_str());
    }
    else if (name.compare("amc") == 0 || name.compare("agent-max-climb") == 0)
    {
        settings.agentMaxClimb = (float)std::atof(value.c_str());
    }
    else if (name.compare("ams") == 0 || name.compare("agent-max-slope") == 0)
    {
        settings.agentMaxSlope = (float)std::atof(value.c_str());
    }
    // Region
    else if (name.compare("rms") == 0 || name.compare("region-min-size") == 0)
    {
        settings.regionMinSize = (float)std::atof(value.c_str());
    }
    else if (name.compare("rmes") == 0 || name.compare("region-merge-size") == 0)
    {
        settings.regionMergeSize = (float)std::atof(value.c_str());
    }

    else if (name.compare("pt") == 0 || name.compare("partition-type") == 0)
    {
        settings.partitionType = std::atoi(value.c_str());
    }
    else if (name.compare("eml") == 0 || name.compare("edge-max-len") == 0)
    {
        settings.edgeMaxLen = (float)std::atof(value.c_str());
    }
    else if (name.compare("eme") == 0 || name.compare("edge-max-error") == 0)
    {
        settings.edgeMaxError = (float)std::atof(value.c_str());
    }
    else if (name.compare("vpp") == 0 || name.compare("verts-per-poly") == 0)
    {
        settings.vertsPerPoly = (float)std::atoi(value.c_str());
    }
    else if (name.compare("dsd") == 0 || name.compare("detail-sample-dist") == 0)
    {
        settings.detailSampleDist = (float)std::atof(value.c_str());
    }
    else if (name.compare("dsmm") == 0 || name.compare("detail-sample-max-error") == 0)
    {
        settings.detailSampleMaxError = (float)std::atoi(value.c_str());
    }
    else if (name.compare("ts") == 0 || name.compare("tile-size") == 0)
    {
        settings.tileSize = (float)std::atoi(value.c_str());
    }
    else if (name.compare("hmsx") == 0 || name.compare("heightmap-scaling-x") == 0)
    {
        settings.hmScaleX = (float)std::atof(value.c_str());
    }
    else if (name.compare("hmsy") == 0 || name.compare("heightmap-scaling-y") == 0)
    {
        settings.hmScaleY = (float)std::atof(value.c_str());
    }
    else if (name.compare("hmsz") == 0 || name.compare("heightmap-scaling-z") == 0)
    {
        settings.hmScaleZ = (float)std::atof(value.c_str());
    }
    else if (name.compare("hmps") == 0 || name.compare("heightmap-patch-size") == 0)
    {
        settings.hmPatchSize = std::atoi(value.c_str());
    }
    else
        std::cout << "Warning: Unknown command line switch " << name << std::endl;
}

static int ParseOptions(int argc, char** argv, BuildSettings& settings)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);
        if (arg[0] != '-' && arg[0] != '/')
            return i;
        arg.erase(0, 1);
        ParseArg(arg, settings);
    }
    return argc;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        ShowUsage();
        return 1;
    }
    BuildContext ctx;
    ctx.verbose_ = verbose;
    BuildSettings settings;
    ConfigFile cfg;
    if (cfg.Load("genavmesh.cfg"))
        std::cout << "Found config file genavmesh.cfg" << std::endl;
    else
        std::cout << "Config file genavmesh.cfg not found" << std::endl;

    // Rasterization
    settings.cellSize = cfg.Get("cell-size", 0.3f);
    settings.cellHeight = cfg.Get("cell-height", 0.2f);
    // Agent
    settings.agentHeight = cfg.Get("agent-height", 2.0f);
    settings.agentRadius = cfg.Get("agent-radius", 0.6f);
    settings.agentMaxClimb = cfg.Get("agent-max-climb", 0.9f);
    settings.agentMaxSlope = cfg.Get("agent-max-slope", 45.0f);
    // Region
    settings.regionMinSize = cfg.Get("region-min-size", 8.0f);
    settings.regionMergeSize = cfg.Get("region-merge-size", 20.0f);
    // Partitioning
    settings.partitionType = cfg.Get<int>("partition-type", 1);
    // Filtering
    // Polygonization
    settings.edgeMaxLen = cfg.Get("edge-max-len", 12.0f);
    settings.edgeMaxError = cfg.Get("edge-max-error", 1.3f);
    settings.vertsPerPoly = (float)cfg.Get<int>("verts-per-poly", 6);
    // Detail Mesh
    settings.detailSampleDist = (float)cfg.Get<int>("details-sample-dist", 6);
    settings.detailSampleMaxError = (float)cfg.Get<int>("details-sample-max-error", 1);
    // Tiling
    settings.tileSize = (float)cfg.Get<int>("tile-size", 32);
    // Heightmap
    settings.hmScaleX = cfg.Get("hm-scale-x", 1.0f);
    settings.hmScaleY = cfg.Get("hm-scale-y", 0.25f);    // Urho3D default
    settings.hmScaleZ = cfg.Get("hm-scale-z", 1.0f);
    settings.hmPatchSize = cfg.Get("hm-patch-size", 32);

    int filesStart = ParseOptions(argc, argv, settings);

    for (int i = filesStart; i < argc; i++)
    {
        InputGeom geom;
        std::string fn(argv[i]);
        std::cout << "Processing file " << fn << std::endl;
        if (!geom.load(&ctx, &settings, fn))
        {
            std::cerr << "Error loading file " << fn << std::endl;
            continue;
        }
        rcVcopy(settings.navMeshBMin, geom.getNavMeshBoundsMin());
        rcVcopy(settings.navMeshBMax, geom.getNavMeshBoundsMax());

        if (!noObj)
        {
            std::string objFile = fn + ".obj";
            if (geom.saveObj(&settings, objFile))
                std::cout << "Saved height map mesh to " << objFile << std::endl;
            else
                std::cerr << "Error saving mesh" << std::endl;
        }

        TileBuilder builder(&ctx);
        builder.m_filterLowHangingObstacles = true;
        builder.m_filterLedgeSpans = true;
        builder.m_filterWalkableLowHeightSpans = true;
        if (!builder.Build(&geom, settings))
        {
            continue;
        }
        std::string navmeshFile = fn + ".navmesh";
        builder.Save(navmeshFile.c_str(), builder.GetNavMesh());
        std::cout << "Created NavMesh " << navmeshFile << std::endl;
    }
    return 0;
}
