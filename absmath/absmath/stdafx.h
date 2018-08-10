// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschließen

#define _USE_MATH_DEFINES
#include <cmath>

#define HAVE_DIRECTX_MATH

// STL
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <list>
#include <mutex>
#include <thread>
#include <chrono>
#include <ostream>
#include <iostream>
#include <sstream>
#include <functional>
#include <condition_variable>
#include <unordered_set>

#include <stdint.h>

#ifdef HAVE_DIRECTX_MATH
#include <DirectXMath.h>
#endif
