// stdafx.cpp : Quelldatei, die nur die Standard-Includes einbindet.
// abserv.pch ist der vorkompilierte Header.
// stdafx.obj enthält die vorkompilierten Typinformationen.

#include "stdafx.h"

#define BASE64_IMPLEMENTATION
#include <base64.h>
#undef BASE64_IMPLEMENTATION

#pragma comment(lib, "PugiXml.lib")
#pragma comment(lib, "abcrypto.lib")
#pragma comment(lib, "Detour.lib")
