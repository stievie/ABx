#pragma once

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschlieﬂen

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdio.h>

#define ASIO_STANDALONE

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

#include "MathConfig.h"

/*
#include <pugixml.hpp>
#pragma warning(push)
#pragma warning(disable: 4592)
#include <asio.hpp>
#pragma warning(pop)

#include <uuid.h>
*/