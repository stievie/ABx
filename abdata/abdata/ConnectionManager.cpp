#include "stdafx.h"
#include "ConnectionManager.h"

void ConnectionManager::start(ConnectionPtr c)
{
	connections_.insert(c);
	c->start();
}

void ConnectionManager::stop(ConnectionPtr c)
{
	connections_.erase(c);
	c->stop();
}

void ConnectionManager::stop_all()
{
	std::for_each(connections_.begin(), connections_.end(),
		std::bind(&Connection::stop, std::placeholders::_1));
	connections_.clear();
}
