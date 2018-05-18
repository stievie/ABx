// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschließen

#include <memory>
#include <iostream>

#define ASIO_STANDALONE

#pragma warning(push)
#pragma warning(disable: 4592)
#include <asio.hpp>
#pragma warning(pop)

#include <uuid.h>

#define SCHEDULER_MINTICKS 10
