#include "stdafx.h"
#include <sa/PragmaWarning.h>

PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4244 4456)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION
PRAGMA_WARNING_POP

#if defined(_MSC_VER)
#pragma comment(lib, "assimp-vc140-mt.lib")
#ifdef _DEBUG
#pragma comment(lib, "zlibstaticd.lib")
#else
#pragma comment(lib, "zlibstatic.lib")
#endif
#endif
