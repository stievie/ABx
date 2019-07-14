// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"

#include <stdio.h>

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <fstream>
#include <functional>
#include <thread>

#include <AB/CommonConfig.h>
#include <base64.h>
#include "DebugConfig.h"

#if !defined(USE_MYSQL) && !defined(USE_PGSQL) && !defined(USE_ODBC) && !defined(USE_SQLITE)
#error "Define at least one database driver"
#endif
