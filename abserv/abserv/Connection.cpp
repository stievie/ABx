#include "stdafx.h"
#include "Connection.h"
#include <thread>


Connection::~Connection()
{
}

std::shared_ptr<Connection> ConnectionManager::CreateConnection(asio::ip::tcp::socket* socket,
    asio::io_service& ioService, std::shared_ptr<ServicePort> servicer)
{
    std::lock_guard<std::recursive_mutex> lock(connectionManagerlock_);
    std::shared_ptr<Connection> connection = std::make_shared<Connection>(socket, ioService, servicer);
    connections_.push_back(connection);

    return connection;
}

void ConnectionManager::ReleaseConnection(std::shared_ptr<Connection> connection)
{
}

void ConnectionManager::CloseAll()
{
}
