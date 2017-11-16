// stdafx.cpp : Quelldatei, die nur die Standard-Includes einbindet.
// genavmesh.pch ist der vorkompilierte Header.
// stdafx.obj enthält die vorkompilierten Typinformationen.

#include "stdafx.h"

// TODO: Auf zusätzliche Header verweisen, die in STDAFX.H
// und nicht in dieser Datei erforderlich sind.

// Assimp
#if defined(_DEBUG)
#   pragma comment(lib, "zlibstaticd.lib")
#else
#   pragma comment(lib, "zlibstatic.lib")
#endif
#pragma comment(lib, "assimp-vc140-mt.lib")
