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
            uint16_t keySize = toInt16(*data_.get(), 1);
            if (keySize <= (uint16_t)maxKeySize_)
            {
                StartReadKey(keySize);
            }
            else
            {
                SendStatusAndRestart(KeyTooBig, "supplied key is too big.Maximum allowed key size is: " + maxKeySize_);
            }
        }
        else
        {
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartReadKey(uint16_t& keySize)
{
    key_.resize(keySize);// .clear();//start fresh.TODO consider cost of this
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(key_), asio::transfer_at_least(keySize),
        [this, self, keySize](const asio::error_code& error, size_t byteTransferred)
    {
        if (!error)
        {
            if ((uint16_t)byteTransferred != keySize)
            {
                SendStatusAndRestart(OtherErrors, "Data sent is not equal to the expected size");
            }
            else
            {
                StartClientRequestedOp();
            }
        }
        else
        {
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartSetDataOperation()
{
    data_.reset(new std::vector<uint8_t>(4));
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(4),
        [this, self](const asio::error_code& error, size_t /* bytes_transferred */)
    {
        if (!error)
        {
            uint32_t size = toInt32(*data_.get(), 0);
            if (size <= maxDataSize_)
            {
                data_.reset(new std::vector<uint8_t>(size));
                asio::async_read(socket_, asio::buffer(*data_.get()), asio::transfer_at_least(size),
                    std::bind(&Connection::HandleReadRawData, self,
                        std::placeholders::_1, std::placeholders::_2, size));
            }
            else
            {
                SendStatusAndRestart(DataTooBig, "The data sent is too big.Maximum data allowed is: " + maxDataSize_);
            }
        }
        else
        {
            connectionManager_.Stop(self);
        }
    });
}

void Connection::StartGetOperation()
{
    data_ = storageProvider_.Get(key_);
    if (data_)
    {

        uint8_t header[] = {
            (uint8_t)Data,
            (uint8_t)data_->size(),
            (uint8_t)(data_->size() >> 8),
            (uint8_t)(data_->size() >> 16),
            (uint8_t)(data_->size() >> 24)
        };//TODO verify this

        std::vector<asio::mutable_buffer> bufs = {
            asio::buffer(header),
            asio::buffer(*data_.get())
        };

        SendResponseAndStart(bufs, data_->size() + 4);
    }
    else
    {
        /*boost::shared_ptr<std::vector<uint8_t>> r;
        data_ = r;*/
        SendStatusAndRestart(NoSuchKey, "Requested data not in cache");
    }
}

void Connection::StartDeleteOperation()
{
    if (storageProvider_.Remove(key_))
    {
        SendStatusAndRestart(Ok, "OK");
    }
    else
    {
        SendStatusAndRestart(OtherErrors, "Supplied key not found in cache");
    }
}

void Connection::Stop()
{
    std::string address = socket_.remote_endpoint().address().to_string() + ":" + std::to_string(socket_.remote_endpoint().port());
    socket_.close();
    std::cout << "Connection Closed: " << address << std::endl;
}

void Connection::HandleReadRawData(const asio::error_code& error,
    size_t bytes_transferred, size_t expected)
{
    if (!error)
    {
        if (bytes_transferred != expected)
        {
            SendStatusAndRestart(OtherErrors, "Data size sent is not equal to data expected");
        }
        else
        {
            storageProvider_.Save(key_, data_);
            SendStatusAndRestart(Ok, "OK");
        }
    }
    else
    {
        connectionManager_.Stop(shared_from_this());
    }
}

void Connection::HandleWriteReqResponse(const asio::error_code& error)
{
    if (!error)
        Start();
    else
        connectionManager_.Stop(shared_from_this());
}

void Connection::StartClientRequestedOp()
{
    switch (opcode_)
    {
    case 0:
        StartSetDataOperation();
        break;
    case 1:
        StartGetOperation();
        break;
    case 2:
        StartDeleteOperation();
        break;
    case 3:
    case 4:
        SendStatusAndRestart(OtherErrors, "Opcode you sent is not valid in this case");
        break;
    default:
    {
        SendStatusAndRestart(OtherErrors, "Opcode you sent does not exist");
    }
    }
}

void Connection::SendStatusAndRestart(ErrorCodes code, std::string message)
{
    data_.reset(new std::vector<uint8_t>);
    data_.get()->push_back(Status);
    data_.get()->push_back(code);
    data_.get()->push_back(0);
    data_.get()->push_back(0);
    data_.get()->push_back(0);

    //push message length
    data_->push_back((unsigned char)message.length());
    for (auto letter : message)
    {
        data_.get()->push_back(letter);
    }
    std::vector<asio::mutable_buffer> bufs = { asio::buffer(*data_.get()) };
    SendResponseAndStart(bufs, data_->size());
}

void Connection::SendResponseAndStart(std::vector<asio::mutable_buffer>& resp, size_t size)
{
    asio::async_write(socket_, resp, asio::transfer_at_least(size), std::bind(&Connection::HandleWriteReqResponse,
        shared_from_this(), std::placeholders::_1));
}

