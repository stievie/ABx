#pragma once

#include <sa/PragmaWarning.h>

PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4592)
#   include <SimpleWeb/server_https.hpp>
PRAGMA_WARNING_POP

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
