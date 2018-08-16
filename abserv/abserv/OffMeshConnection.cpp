#include "stdafx.h"
#include "OffMeshConnection.h"

namespace Navigation {

static constexpr float DEFAULT_RADIUS = 1.0f;

OffMeshConnection::OffMeshConnection() :
    bidirectional_(true),
    radius_(DEFAULT_RADIUS)
{
}

}
