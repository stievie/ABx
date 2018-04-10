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
	void Start(ConnectionPtr c);

	/// Stop the specified connection.
	void Stop(ConnectionPtr c);

	/// Stop all connections.
	void StopAll();

private:

	std::set<ConnectionPtr> connections_;

};