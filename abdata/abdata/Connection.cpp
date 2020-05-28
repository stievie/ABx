/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "Connection.h"
#include <iostream>
#include <algorithm>
#include "ConnectionManager.h"
#include "StorageProvider.h"

Connection::Connection(asio::io_service& io_service, ConnectionManager& manager,
    StorageProvider& storage, size_t maxData, uint16_t maxKey) :
    maxDataSize_(maxData),
    maxKeySize_(maxKey),
    socket_(io_service),
    connectionManager_(manager),
    storageProvider_(storage)
{ }

asio::ip::tcp::socket& Connection::GetSocket()
{
    return socket_;
}

void Connection::Start()
{
    data_.reset(new StorageData(3));
    auto self(shared_from_this());
    asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(3),
        [this, self](const asio::error_code& error, size_t /* bytes_transferred */)
    {
        if (!error)
        {
            opcode_ = static_cast<IO::OpCodes>(data_->at(0));
            const uint16_t keySize = ToInt16(*data_, 1);
            if (keySize <= maxKeySize_)
                StartReadKey(keySize);
            else
                SendStatusAndRestart(IO::ErrorCodes::KeyTooBig, "Supplied key is too big. Maximum allowed key size is: " + std::to_string(maxKeySize_));
        }
        else
        {
            LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartReadKey(uint16_t keySize)
{
    key_.resize(keySize);
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(key_.data_), asio::transfer_at_least(keySize),
        [this, self, keySize](const asio::error_code& error, size_t byteTransferred)
    {
        if (!error)
        {
            if (static_cast<uint16_t>(byteTransferred) != keySize)
                SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Data sent is not equal to the expected size");
            else
                StartClientRequestedOp();
        }
        else
        {
            LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartCreateOperation()
{
    data_.reset(new StorageData(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t bytes_transferred)
    {
        if (!error)
        {
            uint32_t size = ToInt32(*data_, 0);
            if (size == 0)
            {
                LOG_ERROR << "Size = 0, bytes_transferred = " << bytes_transferred << std::endl;
                return;
            }
            if (size <= maxDataSize_)
            {
                data_.reset(new StorageData(size));
                asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleCreateReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
                SendStatusAndRestart(IO::ErrorCodes::DataTooBig, "The data sent is too big. Maximum data allowed is: " + std::to_string(maxDataSize_));
        }
        else
        {
            LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartUpdateDataOperation()
{
    data_.reset(new StorageData(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t bytes_transferred)
    {
        if (!error)
        {
            uint32_t size = ToInt32(*data_, 0);
            if (size == 0)
            {
                LOG_ERROR << "Size = 0, bytes_transferred = " << bytes_transferred << std::endl;
                return;
            }
            if (size <= maxDataSize_)
            {
                data_.reset(new StorageData(size));
                asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleUpdateReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
                SendStatusAndRestart(IO::ErrorCodes::DataTooBig, "The data sent is too big. Maximum data allowed is: " + std::to_string(maxDataSize_));
        }
        else
        {
            LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartReadOperation()
{
    data_.reset(new StorageData(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t bytes_transferred)
    {
        if (!error && bytes_transferred >= 4)
        {
            uint32_t size = ToInt32(*data_, 0);
            if (size == 0)
            {
                LOG_ERROR << "Size = 0, bytes_transferred = " << bytes_transferred << std::endl;
                return;
            }
            if (size <= maxDataSize_)
            {
                data_.reset(new StorageData(size));
                asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleReadReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
                SendStatusAndRestart(IO::ErrorCodes::DataTooBig, "The data sent is too big. Maximum data allowed is: " + std::to_string(maxDataSize_));
        }
        else
        {
            LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartDeleteOperation()
{
    if (storageProvider_.Delete(key_))
        SendStatusAndRestart(IO::ErrorCodes::Ok, "OK");
    else
        SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Supplied key not found in cache");
}

void Connection::StartInvalidateOperation()
{
    if (storageProvider_.Invalidate(key_))
        SendStatusAndRestart(IO::ErrorCodes::Ok, "OK");
    else
        SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Supplied key not found in cache");
}

void Connection::StartPreloadOperation()
{
    if (storageProvider_.Preload(key_))
        SendStatusAndRestart(IO::ErrorCodes::Ok, "OK");
    else
        SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Supplied key not found in cache");
}

void Connection::StartExistsOperation()
{
    data_.reset(new StorageData(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t bytes_transferred)
    {
        if (!error)
        {
            uint32_t size = ToInt32(*data_, 0);
            if (size == 0)
            {
                LOG_ERROR << "Size = 0, bytes_transferred = " << bytes_transferred << std::endl;
                return;
            }
            if (size <= maxDataSize_)
            {
                data_.reset(new StorageData(size));
                asio::async_read(socket_, asio::buffer(*data_), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleExistsReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
                SendStatusAndRestart(IO::ErrorCodes::DataTooBig, "The data sent is too big. Maximum data allowed is: " + std::to_string(maxDataSize_));
        }
        else
        {
            LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartClearOperation()
{
    if (storageProvider_.Clear(key_))
        SendStatusAndRestart(IO::ErrorCodes::Ok, "OK");
    else
        SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Other Error");
}

void Connection::Stop()
{
    socket_.close();
}

void Connection::HandleUpdateReadRawData(const asio::error_code& error,
    size_t bytes_transferred, size_t expected)
{
    // Network/main thread
    if (!error)
    {
        if (bytes_transferred != expected)
            SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Data size sent is not equal to data expected");
        else
        {
            bool res = false;
            {
                std::scoped_lock lock(lock_);
                res = storageProvider_.Update(key_, data_);
            }
            if (res)
                SendStatusAndRestart(IO::ErrorCodes::Ok, "OK");
            else
                SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Error");
        }
    }
    else
    {
        LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        connectionManager_.Stop(shared_from_this());
    }
}

void Connection::HandleCreateReadRawData(const asio::error_code& error,
    size_t bytes_transferred, size_t expected)
{
    // Network/main thread
    if (!error)
    {
        if (bytes_transferred != expected)
            SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Data size sent is not equal to data expected");
        else
        {
            bool res = false;
            {
                std::scoped_lock lock(lock_);
                res = storageProvider_.Create(key_, data_);
            }
            if (res)
                SendStatusAndRestart(IO::ErrorCodes::Ok, "OK");
            else
                SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Error");
        }
    }
    else
    {
        LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        connectionManager_.Stop(shared_from_this());
    }
}

void Connection::HandleReadReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected)
{
    // Network/main thread
    if (error)
    {
        LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        connectionManager_.Stop(shared_from_this());
        return;
    }
    if (!data_)
    {
        SendStatusAndRestart(IO::ErrorCodes::NoSuchKey, "Requested data not in cache");
        return;
    }
    if (bytes_transferred != expected)
    {
        SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Data size sent is not equal to data expected");
        return;
    }

    bool res = false;
    {
        std::scoped_lock lock(lock_);
        res = storageProvider_.Read(key_, data_);
    }

    if (!res)
    {
        SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Error");
        return;
    }

    uint8_t header[] = {
        static_cast<uint8_t>(IO::OpCodes::Data),
        static_cast<uint8_t>(data_->size()),
        static_cast<uint8_t>(data_->size() >> 8),
        static_cast<uint8_t>(data_->size() >> 16),
        static_cast<uint8_t>(data_->size() >> 24)
    };

    std::vector<asio::mutable_buffer> bufs = {
        asio::buffer(header),
        asio::buffer(*data_.get())
    };
    SendResponseAndStart(bufs, data_->size() + 4);
}

void Connection::HandleExistsReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected)
{
    // Network/main thread
    if (!error)
    {
        if (bytes_transferred != expected)
            SendStatusAndRestart(IO::ErrorCodes::OtherErrors, "Data size sent is not equal to data expected");
        else
        {
            bool res = false;
            {
                std::scoped_lock lock(lock_);
                res = storageProvider_.Exists(key_, data_);
            }
            if (res)
                SendStatusAndRestart(IO::ErrorCodes::Ok, "OK");
            else
                SendStatusAndRestart(IO::ErrorCodes::NotExists, "Record does not exist");
        }
    }
    else
    {
        LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        connectionManager_.Stop(shared_from_this());
    }
}

void Connection::HandleWriteReqResponse(const asio::error_code& error)
{
    if (!error)
        Start();
    else
    {
        LOG_ERROR << "Network (" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        connectionManager_.Stop(shared_from_this());
    }
}

void Connection::StartClientRequestedOp()
{
    // All this executed in dispatcher thread
    switch (opcode_)
    {
    case IO::OpCodes::Create:
        AddTask(&Connection::StartCreateOperation);
        break;
    case IO::OpCodes::Update:
        AddTask(&Connection::StartUpdateDataOperation);
        break;
    case IO::OpCodes::Read:
        AddTask(&Connection::StartReadOperation);
        break;
    case IO::OpCodes::Delete:
        AddTask(&Connection::StartDeleteOperation);
        break;
    case IO::OpCodes::Invalidate:
        AddTask(&Connection::StartInvalidateOperation);
        break;
    case IO::OpCodes::Preload:
        AddTask(&Connection::StartPreloadOperation);
        break;
    case IO::OpCodes::Exists:
        AddTask(&Connection::StartExistsOperation);
        break;
    case IO::OpCodes::Clear:
        AddTask(&Connection::StartClearOperation);
        break;
    case IO::OpCodes::Status:
    case IO::OpCodes::Data:
        LOG_ERROR << "Status and Data OP Codes are invalid here" << std::endl;
        AddTask(&Connection::SendStatusAndRestart, IO::ErrorCodes::OtherErrors, "Invalid Opcode");
        break;
    default:
    {
        const auto ep = socket_.remote_endpoint();
        LOG_ERROR << "Invalid OP Code " << static_cast<int>(opcode_) << " from " <<
            ep.address().to_string() << ":" << ep.port() << std::endl;
        AddTask(&Connection::SendStatusAndRestart, IO::ErrorCodes::OtherErrors, "Invalid Opcode");
        break;
    }
    }
}

void Connection::SendStatusAndRestart(IO::ErrorCodes code, const std::string& message)
{
    data_.reset(new StorageData);
    data_->push_back(static_cast<uint8_t>(IO::OpCodes::Status));
    data_->push_back(static_cast<uint8_t>(code));
    data_->push_back(0);
    data_->push_back(0);
    data_->push_back(0);

    //push message length
    data_->push_back(std::min(static_cast<uint8_t>(255), static_cast<uint8_t>(message.length())));
    size_t len = 0;
    for (const auto& letter : message)
    {
        data_->push_back(static_cast<uint8_t>(letter));
        ++len;
        if (len >= 255)
            break;
    }
    std::vector<asio::mutable_buffer> bufs = { asio::buffer(*data_) };
    SendResponseAndStart(bufs, data_->size());
}

void Connection::SendResponseAndStart(std::vector<asio::mutable_buffer>& resp, size_t size)
{
    asio::async_write(socket_, resp, asio::transfer_at_least(size),
        std::bind(&Connection::HandleWriteReqResponse,
            shared_from_this(), std::placeholders::_1));
}

