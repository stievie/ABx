#include "stdafx.h"
#include "ConnectionManager.h"

void ConnectionManager::Start(ConnectionPtr c)
{
	connections_.insert(c);
	c->Start();
}

void ConnectionManager::Stop(ConnectionPtr c)
{
	connections_.erase(c);
	c->Stop();
}

void ConnectionManager::StopAll()
{
	std::for_each(connections_.begin(), connections_.end(),
		std::bind(&Connection::Stop, std::placeholders::_1));
	connections_.clear();
}
