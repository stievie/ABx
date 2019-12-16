#pragma once

#include "Node.h"
#include "Filter.h"
#include "Condition.h"
#include <iostream>
#include "BevaviorCache.h"
#include <CleanupNs.h>

namespace AI {

void DumpTree(std::ostream& stream, const Node& node);
void DumpCache(std::ostream& stream, const BevaviorCache& cache);

}
