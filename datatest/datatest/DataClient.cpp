#include "stdafx.h"
#include "DataClient.h"

DataClient::DataClient(asio::io_service& io_service) :
    resolver_(io_service),
    socket_(io_service)
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
    InternalConnect();
}

std::shared_ptr<std::vector<uint8_t>> DataClient::Read(const std::string& key)
{
    uint8_t ksize1 = static_cast<uint8_t>(key.size());
    uint8_t ksize2 = static_cast<uint8_t>(key.size() >> 8);
    uint8_t header[] = { OpCodes::Read, ksize1, ksize2 };
    socket_.send(asio::buffer(header));

    socket_.send(asio::buffer(key));
    std::vector<uint8_t> dataheader(5);
    size_t read = asio::read(socket_, asio::buffer(dataheader, 5), asio::transfer_at_least(5));

    const int size = toInt32(dataheader, 1);
    //TODO handle message statuses
    std::vector<uint8_t> data(size);
    size_t read2 = asio::read(socket_, asio::buffer(data), asio::transfer_at_least(size));

    return std::shared_ptr<std::vector<uint8_t>>(&data);
}

bool DataClient::Delete(const std::string& key)
{
    uint8_t ksize1 = static_cast<uint8_t>(key.size());
    uint8_t ksize2 = static_cast<uint8_t>(key.size() >> 8);
    uint8_t header[] = { OpCodes::Delete, ksize1, ksize2 };
    socket_.send(asio::buffer(header));

    socket_.send(asio::buffer(key));

    uint8_t dataheader[5];
    size_t read = asio::read(socket_, asio::buffer(dataheader, 5), asio::transfer_at_least(5));

    const int code = dataheader[4] << 24 | dataheader[3] << 16 | dataheader[2] << 8 | dataheader[1];
    int size = 2;

    std::vector<uint8_t> data(size);
    size_t read2 = asio::read(socket_, asio::buffer(data), asio::transfer_at_least(size));

    return true;
}

bool DataClient::Update(const std::string& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    uint8_t ksize1 = static_cast<uint8_t>(key.size());
    uint8_t ksize2 = static_cast<uint8_t>(key.size() >> 8);
    uint8_t header[] = { OpCodes::Update, ksize1, ksize2 };
    socket_.send(asio::buffer(header));

    socket_.send(asio::buffer(key));

    uint8_t dsize4 = static_cast<uint8_t>(data->size() >> 24);
    uint8_t dsize3 = static_cast<uint8_t>(data->size() >> 16);
    uint8_t dsize2 = static_cast<uint8_t>(data->size() >> 8);
    uint8_t dsize1 = static_cast<uint8_t>(data->size());

    uint8_t data_header[] = { dsize1, dsize2, dsize3, dsize4 };
    socket_.send(asio::buffer(data_header));

    socket_.send(asio::buffer(*data.get()));

    uint8_t result[128];
    size_t read = socket_.read_some(asio::buffer(result));

    return true;
}

uint32_t DataClient::Create(const std::string& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    uint8_t ksize1 = static_cast<uint8_t>(key.size());
    uint8_t ksize2 = static_cast<uint8_t>(key.size() >> 8);
    uint8_t header[] = { OpCodes::Create, ksize1, ksize2 };
    socket_.send(asio::buffer(header));

    socket_.send(asio::buffer(key));

    uint8_t dsize4 = static_cast<uint8_t>(data->size() >> 24);
    uint8_t dsize3 = static_cast<uint8_t>(data->size() >> 16);
    uint8_t dsize2 = static_cast<uint8_t>(data->size() >> 8);
    uint8_t dsize1 = static_cast<uint8_t>(data->size());

    uint8_t data_header[] = { dsize1, dsize2, dsize3, dsize4 };
    socket_.send(asio::buffer(data_header));

    socket_.send(asio::buffer(*data.get()));

    uint8_t result[128];
    size_t read = socket_.read_some(asio::buffer(result));

    return 0;
}

void DataClient::InternalConnect()
{
    asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), host_, std::to_string(port_));
    asio::ip::tcp::resolver::iterator endpoint = resolver_.resolve(query);
    asio::connect(socket_, endpoint);
}
