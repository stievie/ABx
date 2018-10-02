// stdafx.cpp : Quelldatei, die nur die Standard-Includes einbindet.
// abdata.pch ist der vorkompilierte Header.
// stdafx.obj enthält die vorkompilierten Typinformationen.

#include "stdafx.h"

// TODO: Auf zusätzliche Header verweisen, die in STDAFX.H
// und nicht in dieser Datei erforderlich sind.

#define BASE64_IMPLEMENTATION
#include <base64.h>
#undef BASE64_IMPLEMENTATION
