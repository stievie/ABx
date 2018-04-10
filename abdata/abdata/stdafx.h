// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <map>

#define ASIO_STANDALONE

#pragma warning(push)
#pragma warning(disable: 4592)
#include <asio.hpp>
#pragma warning(pop)

#define USE_MYSQL
#define USE_PGSQL
#define USE_ODBC

// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
