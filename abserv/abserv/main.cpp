// Entry point

#include "stdafx.h"
#include "Application.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#   define CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#endif

#include "DebugNew.h"

int main(int argc, char** argv)
{
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Application app;
    if (!app.Initialize(argc, argv))
        return EXIT_FAILURE;

    app.Run();
    return EXIT_SUCCESS;
}
