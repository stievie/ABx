#include "stdafx.h"
#include "Connection.h"
#include <iostream>
#include "ConnectionManager.h"
#include "StorageProvider.h"

Connection::Connection(asio::io_service& io_service, ConnectionManager& manager,
    StorageProvider& storage, size_t maxData, size_t maxKey) :
    socket_(io_service),
    connectionManager_(manager),
    storageProvider_(storage),
    maxDataSize_(maxData),
    maxKeySize_(maxKey),
    opcode_(0)
{ }

asio::ip::tcp::socket& Connection::socket()
{
    return socket_;
}

void Connection::Start()
{
    data_.reset(new std::vector<uint8_t>(3));
    auto self(shared_from_this());
    asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(3),
        [this, self](const asio::error_code& error, size_t /* bytes_transferred */)
    {
        if (!error)
        {
            opcode_ = data_->at(0);
            uint16_t keySize = ToInt16(*data_.get(), 1);
            if (keySize <= (uint16_t)maxKeySize_)
                StartReadKey(keySize);
            else
                SendStatusAndRestart(KeyTooBig, "Supplied key is too big. Maximum allowed key size is: " + maxKeySize_);
        }
        else
            connectionManager_.Stop(self);
    });
}

void Connection::StartReadKey(uint16_t& keySize)
{
    key_.resize(keySize);
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(key_.data_), asio::transfer_at_least(keySize),
        [this, self, keySize](const asio::error_code& error, size_t byteTransferred)
    {
        if (!error)
        {
            if ((uint16_t)byteTransferred != keySize)
                SendStatusAndRestart(OtherErrors, "Data sent is not equal to the expected size");
            else
                StartClientRequestedOp();
        }
        else
            connectionManager_.Stop(self);
    });
}

void Connection::StartCreateOperation()
{
    data_.reset(new std::vector<uint8_t>(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t /* bytes_transferred */)
    {
        if (!error)
        {
            uint32_t size = ToInt32(*data_.get(), 0);
            if (size <= maxDataSize_)
            {
                data_.reset(new std::vector<uint8_t>(size));
                asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleCreateReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
                SendStatusAndRestart(DataTooBig, "The data sent is too big. Maximum data allowed is: " + maxDataSize_);
        }
        else
            connectionManager_.Stop(self);
    });
}

void Connection::StartUpdateDataOperation()
{
    data_.reset(new std::vector<uint8_t>(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t /* bytes_transferred */)
    {
        if (!error)
        {
            uint32_t size = ToInt32(*data_.get(), 0);
            if (size <= maxDataSize_)
            {
                data_.reset(new std::vector<uint8_t>(size));
                asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleUpdateReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
                SendStatusAndRestart(DataTooBig, "The data sent is too big. Maximum data allowed is: " + maxDataSize_);
        }
        else
            connectionManager_.Stop(self);
    });
}

void Connection::StartReadOperation()
{
    data_.reset(new std::vector<uint8_t>(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t bytes_transferred)
    {
        if (!error && bytes_transferred >= 4)
        {
            uint32_t size = ToInt32(*data_.get(), 0);
            if (size <= maxDataSize_)
            {
                data_.reset(new std::vector<uint8_t>(size));
                asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleReadReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
                SendStatusAndRestart(DataTooBig, "The data sent is too big. Maximum data allowed is: " + maxDataSize_);
        }
        else
            connectionManager_.Stop(self);
    });
}

void Connection::StartDeleteOperation()
{
    if (storageProvider_.Delete(key_))
        SendStatusAndRestart(Ok, "OK");
    else
        SendStatusAndRestart(OtherErrors, "Supplied key not found in cache");
}

void Connection::StartInvalidateOperation()
{
    if (storageProvider_.Invalidate(key_))
        SendStatusAndRestart(Ok, "OK");
    else
        SendStatusAndRestart(OtherErrors, "Supplied key not found in cache");
}

void Connection::StartPreloadOperation()
{
    if (storageProvider_.Preload(key_))
        SendStatusAndRestart(Ok, "OK");
    else
        SendStatusAndRestart(OtherErrors, "Supplied key not found in cache");
}

void Connection::StartExistsOperation()
{
    data_.reset(new std::vector<uint8_t>(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t /* bytes_transferred */)
    {
        if (!error)
        {
            uint32_t size = ToInt32(*data_.get(), 0);
            if (size <= maxDataSize_)
            {
                data_.reset(new std::vector<uint8_t>(size));
                asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleExistsReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
                SendStatusAndRestart(DataTooBig, "The data sent is too big. Maximum data allowed is: " + maxDataSize_);
        }
        else
            connectionManager_.Stop(self);
    });
}

void Connection::Stop()
{
    socket_.close();
}

void Connection::HandleUpdateReadRawData(const asio::error_code& error,
    size_t bytes_transferred, size_t expected)
{
    if (!error)
    {
        if (bytes_transferred != expected)
            SendStatusAndRestart(OtherErrors, "Data size sent is not equal to data expected");
        else
        {
            if (storageProvider_.Update(key_, data_))
                SendStatusAndRestart(Ok, "OK");
            else
                SendStatusAndRestart(OtherErrors, "Error");
        }
    }
    else
        connectionManager_.Stop(shared_from_this());
}

void Connection::HandleCreateReadRawData(const asio::error_code& error,
    size_t bytes_transferred, size_t expected)
{
    if (!error)
    {
        if (bytes_transferred != expected)
            SendStatusAndRestart(OtherErrors, "Data size sent is not equal to data expected");
        else
        {
            if (storageProvider_.Create(key_, data_))
                SendStatusAndRestart(Ok, "OK");
            else
                SendStatusAndRestart(OtherErrors, "Error");
        }
    }
    else
        connectionManager_.Stop(shared_from_this());
}

void Connection::HandleReadReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected)
{
    if (error)
    {
        connectionManager_.Stop(shared_from_this());
        return;
    }
    if (!data_)
    {
        SendStatusAndRestart(NoSuchKey, "Requested data not in cache");
        return;
    }
    if (bytes_transferred != expected)
    {
        SendStatusAndRestart(OtherErrors, "Data size sent is not equal to data expected");
        return;
    }

    if (!storageProvider_.Read(key_, data_))
    {
        SendStatusAndRestart(OtherErrors, "Error");
        return;
    }

    uint8_t header[] = {
        (uint8_t)OpCodes::Data,
        (uint8_t)data_->size(),
        (uint8_t)(data_->size() >> 8),
        (uint8_t)(data_->size() >> 16),
        (uint8_t)(data_->size() >> 24)
    };

    std::vector<asio::mutable_buffer> bufs = {
        asio::buffer(header),
        asio::buffer(*data_.get())
    };
    SendResponseAndStart(bufs, data_->size() + 4);
}

void Connection::HandleWriteReqResponse(const asio::error_code& error)
{
    if (!error)
        Start();
    else
        connectionManager_.Stop(shared_from_this());
}

void Connection::HandleExistsReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected)
{
    if (!error)
    {
        if (bytes_transferred != expected)
            SendStatusAndRestart(OtherErrors, "Data size sent is not equal to data expected");
        else
        {
            if (storageProvider_.Exists(key_, data_))
                SendStatusAndRestart(Ok, "OK");
            else
                SendStatusAndRestart(NotExists, "Record does not exist");
        }
    }
    else
        connectionManager_.Stop(shared_from_this());
}

void Connection::StartClientRequestedOp()
{
    // All this executed in dispatcher thread
    switch (opcode_)
    {
    case OpCodes::Create:
        AddTask(&Connection::StartCreateOperation);
        break;
    case OpCodes::Update:
        AddTask(&Connection::StartUpdateDataOperation);
        break;
    case OpCodes::Read:
        AddTask(&Connection::StartReadOperation);
        break;
    case OpCodes::Delete:
        AddTask(&Connection::StartDeleteOperation);
        break;
    case OpCodes::Invalidate:
        AddTask(&Connection::StartInvalidateOperation);
        break;
    case OpCodes::Preload:
        AddTask(&Connection::StartPreloadOperation);
        break;
    case OpCodes::Exists:
        AddTask(&Connection::StartExistsOperation);
        break;
    case OpCodes::Status:
    case OpCodes::Data:
        AddTask(&Connection::SendStatusAndRestart, OtherErrors, "Invalid Opcode");
        break;
    default:
        AddTask(&Connection::SendStatusAndRestart, OtherErrors, "Invalid Opcode");
        break;
    }
}

void Connection::SendStatusAndRestart(ErrorCodes code, const std::string& message)
{
    data_.reset(new std::vector<uint8_t>);
    data_->push_back(OpCodes::Status);
    data_->push_back(code);
    data_->push_back(0);
    data_->push_back(0);
    data_->push_back(0);

    //push message length
    data_->push_back((unsigned char)message.length());
    for (const auto& letter : message)
    {
        data_->push_back(letter);
    }
    std::vector<asio::mutable_buffer> bufs = { asio::buffer(*data_.get()) };
    SendResponseAndStart(bufs, data_->size());
}

void Connection::SendResponseAndStart(std::vector<asio::mutable_buffer>& resp, size_t size)
{
    asio::async_write(socket_, resp, asio::transfer_at_least(size),
        std::bind(&Connection::HandleWriteReqResponse,
            shared_from_this(), std::placeholders::_1));
}

