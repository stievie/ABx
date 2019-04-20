#pragma once

namespace Client {

enum class ConnectionError
{
    None = 0,
    ResolveError,
    WriteError,
    ConnectError,
    ReceiveError,
    ConnectTimeout,
    ReadTimeout,
    WriteTimeout,
};

}