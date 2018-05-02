#include "stdafx.h"
#include "Server.h"
#include <iostream>
#include "ConnectionManager.h"
#include "Connection.h"
#include "Logger.h"
#include "Scheduler.h"

Server::Server(asio::io_service& io_service, uint32_t ip,
    uint16_t port, size_t maxCacheSize, bool readonly) :
    io_service_(io_service),
    acceptor_(
        io_service,
        asio::ip::tcp::endpoint(asio::ip::address(asio::ip::address_v4(ip)), port)
    ),
    running_(false),
    storageProvider_(maxCacheSize, readonly),
	maxDataSize_(MAX_DATA_SIZE),
    maxKeySize_(MAX_KEY_SIZE)
{
    acceptor_.set_option(asio::ip::tcp::no_delay(true));
	StartAccept();
    running_ = true;
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(LOG_ROTATE_INTERVAL, std::bind(&Server::LogRotateTask, this))
    );
}

Server::~Server()
{
    if (running_)
        Shutdown();
}

void Server::Shutdown()
{
    connectionManager_.StopAll();
    storageProvider_.Shutdown();
    io_service_.stop();
    running_ = false;
}

void Server::LogRotateTask()
{
    if (IO::Logger::Instance().logDir_.empty())
        return;

    IO::Logger::Instance().Close();

    if (running_)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(LOG_ROTATE_INTERVAL, std::bind(&Server::LogRotateTask, this))
        );
    }
}

void Server::StartAccept()
{
	newConnection_.reset(new Connection(io_service_, connectionManager_, storageProvider_, maxDataSize_, maxKeySize_));
	acceptor_.async_accept(newConnection_->socket(),
        std::bind(&Server::HandleAccept, this,
            std::placeholders::_1));
}

void Server::HandleAccept(const asio::error_code& error)
{
	if (!error)
	{
		auto endp = newConnection_.get()->socket().remote_endpoint();
        LOG_INFO << "Connection from " << endp.address() << ":" << endp.port() << std::endl;
		connectionManager_.Start(newConnection_);
	}
	StartAccept();
}
