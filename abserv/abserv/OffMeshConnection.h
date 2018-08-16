#pragma once

#include "Vector3.h"

namespace Navigation {

/// Things such as teleporters, elevaters, area transitions, and so forth, that
/// connect disconnected navmeshes together, and path searching will handle off-mesh
/// connections easily.
class OffMeshConnection
{
public:
    OffMeshConnection();
    ~OffMeshConnection() = default;

    /// Radius.
    float radius_;
    /// Bidirectional flag.
    bool bidirectional_;
    Math::Vector3 start_;
    Math::Vector3 end_;
};

}
