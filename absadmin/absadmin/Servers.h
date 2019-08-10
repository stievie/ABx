#pragma once

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4592)
#endif
#include <SimpleWeb/server_https.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
