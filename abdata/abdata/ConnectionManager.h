#pragma once

#include <set>
#include "Connection.h"

class ConnectionManager
{
public:
    ConnectionManager() = default;
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    /// Add the specified connection to the manager and start it.
	void Start(std::shared_ptr<Connection> c);

	/// Stop the specified connection.
	void Stop(std::shared_ptr<Connection> c);

	/// Stop all connections.
	void StopAll();

private:
	std::set<std::shared_ptr<Connection>> connections_;
};