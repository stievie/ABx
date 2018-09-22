#include "stdafx.h"
#include "DataClient.h"
#include <chrono>
#include <thread>

namespace IO {

DataClient::DataClient(asio::io_service& io_service) :
    resolver_(io_service),
    socket_(io_service),
    connected_(false)
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
    TryConnect(false);
}

bool DataClient::MakeRequest(OpCodes opCode, const DataKey& key, std::vector<uint8_t>& data)
{
    uint8_t ksize1 = static_cast<uint8_t>(key.size());
    uint8_t ksize2 = static_cast<uint8_t>(key.size() >> 8);
    const uint8_t header[] = { opCode, ksize1, ksize2 };
    if (!TryWrite(asio::buffer(header)))
        return false;

    if (!TryWrite(asio::buffer(key.data_)))
        return false;

    uint8_t dsize4 = static_cast<uint8_t>(data.size() >> 24);
    uint8_t dsize3 = static_cast<uint8_t>(data.size() >> 16);
    uint8_t dsize2 = static_cast<uint8_t>(data.size() >> 8);
    uint8_t dsize1 = static_cast<uint8_t>(data.size());

    const uint8_t data_header[] = { dsize1, dsize2, dsize3, dsize4 };
    if (!TryWrite(asio::buffer(data_header)))
        return false;

    if (!TryWrite(asio::buffer(data)))
        return false;

    std::vector<uint8_t> dataheader(5);
    /* size_t read = */ asio::read(socket_, asio::buffer(dataheader, 5), asio::transfer_at_least(5));
    if (dataheader[0] == OpCodes::Status)
    {
        // Does not return data
        uint8_t result[128];
        // Read the rest of the status
        socket_.read_some(asio::buffer(result));
        return dataheader[1] == ErrorCodes::Ok;
    }

    // If we are here it returns data
    const int size = ToInt32(dataheader, 1);
    data.resize(size);
    size_t read2 = asio::read(socket_, asio::buffer(data), asio::transfer_at_least(size));
    if (read2 != size)
        return false;

    return true;
}

bool DataClient::MakeRequestNoData(OpCodes opCode, const DataKey& key)
{
    uint8_t ksize1 = static_cast<uint8_t>(key.size());
    uint8_t ksize2 = static_cast<uint8_t>(key.size() >> 8);
    const uint8_t header[] = { opCode, ksize1, ksize2 };
    if (!TryWrite(asio::buffer(header)))
        return false;

    if (!TryWrite(asio::buffer(key.data_)))
        return false;

    uint8_t result[128];
    size_t read = socket_.read_some(asio::buffer(result));
    if (read < 2)
        return false;

    if (result[0] == OpCodes::Status && result[1] == ErrorCodes::Ok)
        return true;
    return false;
}

bool DataClient::ReadData(const DataKey& key, std::vector<uint8_t>& data)
{
    return MakeRequest(OpCodes::Read, key, data);
}

bool DataClient::DeleteData(const DataKey& key)
{
    return MakeRequestNoData(OpCodes::Delete, key);
}

bool DataClient::ExistsData(const DataKey& key, std::vector<uint8_t>& data)
{
    return MakeRequest(OpCodes::Exists, key, data);
}

bool DataClient::UpdateData(const DataKey& key, std::vector<uint8_t>& data)
{
    return MakeRequest(OpCodes::Update, key, data);
}

bool DataClient::CreateData(const DataKey& key, std::vector<uint8_t>& data)
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

bool DataClient::TryConnect(bool force)
{
    if (force)
    {
        connected_ = false;
        socket_.close();
    }
    // Try for 1 seconds, 10 tries
    int tries = 0;
    while (!connected_ && tries < 10)
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
