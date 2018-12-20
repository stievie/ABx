// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include <stdio.h>

#define ASIO_STANDALONE
#define HAVE_DIRECTX_MATH

#define AB_UNUSED(P) (void)(P)

#include <cassert>

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

#define WRITE_MINIBUMP

#include <pugixml.hpp>
#pragma warning(push)
#pragma warning(disable: 4592)
#include <asio.hpp>
#pragma warning(pop)

#include <uuid.h>
