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

#include "stdafx.h"
#include "DataClient.h"
#include <chrono>
#include <thread>

namespace IO {

DataClient::DataClient(asio::io_service& io_service) :
    socket_(io_service),
    resolver_(io_service)
{
}

DataClient::~DataClient()
{
    socket_.close();
}

void DataClient::Connect(const std::string& host, uint16_t port)
{
    host_ = host;
    port_ = port;
    // First time connect, try longer other servers may not be up yet.
    TryConnect(false, 100);
}

bool DataClient::MakeRequest(OpCodes opCode, const DataKey& key, DataBuff& data)
{
    const uint8_t ksize1 = static_cast<uint8_t>(key.size());
    const uint8_t ksize2 = static_cast<uint8_t>(key.size() >> 8);
    const uint8_t header[] = { static_cast<uint8_t>(opCode), ksize1, ksize2 };
    // All data must be sent at once. Multiple threads must not send different requests at the same time.
    std::scoped_lock lock(lock_);
    if (!TryWrite(asio::buffer(header)))
        return false;

    if (!TryWrite(asio::buffer(key.data_)))
        return false;

    const uint8_t dsize4 = static_cast<uint8_t>(data.size() >> 24);
    const uint8_t dsize3 = static_cast<uint8_t>(data.size() >> 16);
    const uint8_t dsize2 = static_cast<uint8_t>(data.size() >> 8);
    const uint8_t dsize1 = static_cast<uint8_t>(data.size());

    const uint8_t data_header[] = { dsize1, dsize2, dsize3, dsize4 };
    if (!TryWrite(asio::buffer(data_header)))
        return false;

    if (!TryWrite(asio::buffer(data)))
        return false;

    std::vector<uint8_t> dataheader(5);
    /* size_t read = */ asio::read(socket_, asio::buffer(dataheader, 5), asio::transfer_at_least(5));
    if (static_cast<OpCodes>(dataheader[0]) == OpCodes::Status)
    {
        // Does not return data
        uint8_t result[128];
        // Read the rest of the status
        socket_.read_some(asio::buffer(result));
        return dataheader[1] == static_cast<unsigned char>(ErrorCodes::Ok);
    }

    // If we are here it returns data
    const size_t size = static_cast<size_t>(ToInt32(dataheader, 1));
    data.resize(size);
    const size_t read2 = asio::read(socket_, asio::buffer(data), asio::transfer_at_least(size));
    if (read2 != size)
        return false;

    return true;
}

bool DataClient::MakeRequestNoData(OpCodes opCode, const DataKey& key)
{
    const uint8_t ksize1 = static_cast<uint8_t>(key.size());
    const uint8_t ksize2 = static_cast<uint8_t>(key.size() >> 8);
    const uint8_t header[] = { static_cast<uint8_t>(opCode), ksize1, ksize2 };
    // All data must be sent at once. Multiple threads must not send different requests at the same time.
    std::scoped_lock lock(lock_);
    if (!TryWrite(asio::buffer(header)))
        return false;

    if (!TryWrite(asio::buffer(key.data_)))
        return false;

    uint8_t result[128];
    const size_t read = socket_.read_some(asio::buffer(result));
    if (read < 2)
        return false;

    if (static_cast<OpCodes>(result[0]) == OpCodes::Status && static_cast<ErrorCodes>(result[1]) == ErrorCodes::Ok)
        return true;
    return false;
}

bool DataClient::ReadData(const DataKey& key, DataBuff& data)
{
    return MakeRequest(OpCodes::Read, key, data);
}

bool DataClient::DeleteData(const DataKey& key)
{
    return MakeRequestNoData(OpCodes::Delete, key);
}

bool DataClient::ExistsData(const DataKey& key, DataBuff& data)
{
    return MakeRequest(OpCodes::Exists, key, data);
}

bool DataClient::UpdateData(const DataKey& key, DataBuff& data)
{
    return MakeRequest(OpCodes::Update, key, data);
}

bool DataClient::CreateData(const DataKey& key, DataBuff& data)
{
    return MakeRequest(OpCodes::Create, key, data);
}

bool DataClient::PreloadData(const DataKey& key)
{
    return MakeRequestNoData(OpCodes::Preload, key);
}

bool DataClient::InvalidateData(const DataKey& key)
{
    return MakeRequestNoData(OpCodes::Invalidate, key);
}

bool DataClient::Clear()
{
    // Need a key
    static const uuids::uuid guid = uuids::uuid_system_generator{}();
    static DataKey key("__DUMMY__", guid);
    return MakeRequestNoData(OpCodes::Clear, key);
}

void DataClient::InternalConnect()
{
    if (connected_)
        return;

    const asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), host_, std::to_string(port_));
    asio::ip::tcp::resolver::iterator endpoint = resolver_.resolve(query);
    asio::error_code error;
    socket_.connect(*endpoint, error);
    if (!error)
        connected_ = true;
}

bool DataClient::TryConnect(bool force, unsigned numTries /* = 10 */)
{
    if (force)
    {
        connected_ = false;
        socket_.close();
    }
    // By default try for 1 second, 10 tries
    unsigned tries = 0;
    while (!connected_ && tries < numTries)
    {
        ++tries;
        InternalConnect();
        if (connected_)
            return true;
        if (tries >= 10)
            return false;

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
    return connected_;
}

}
