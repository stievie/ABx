#pragma once

#pragma warning( push )
#pragma warning( disable : 4100)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

using namespace Urho3D;

/// User defined events
namespace AbEvents
{
    /// Load next level
    static const StringHash E_SET_LEVEL = StringHash("Set levels");
}
