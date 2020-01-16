#pragma once

#include <set>
#include "Connection.h"
#include <sa/Noncopyable.h>

class ConnectionManager
{
    NON_COPYABLE(ConnectionManager)
public:
    ConnectionManager() = default;

    /// Add the specified connection to the manager and start it.
    void Start(std::shared_ptr<Connection> c);

    /// Stop the specified connection.
    void Stop(std::shared_ptr<Connection> c);

    /// Stop all connections.
    void StopAll();

private:
    std::set<std::shared_ptr<Connection>> connections_;
};
