// stdafx.cpp : Quelldatei, die nur die Standard-Includes einbindet.
// libabclient.pch ist der vorkompilierte Header.
// stdafx.obj enthält die vorkompilierten Typinformationen.

#include "stdafx.h"

// TODO: Auf zusätzliche Header verweisen, die in STDAFX.H
// und nicht in dieser Datei erforderlich sind.

#ifdef _MSC_VER
#pragma comment(lib, "abcrypto.lib")
#if defined(_DEBUG)
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#else
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif
#endif
