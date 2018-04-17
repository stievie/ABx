#pragma once

enum OpCodes
{
    Create = 0,
    Update = 1,
    Read = 2,
    Delete = 3,
    // Responses
    Status,
    Data
};

enum ErrorCodes : uint8_t
{
    Ok,
    NoSuchKey,
    KeyTooBig,
    DataTooBig,
    OtherErrors
};

class DataClient
{
public:
    explicit DataClient(asio::io_service& io_service);
    ~DataClient();

    void Connect(const std::string& host, uint16_t port);

    std::shared_ptr<std::vector<uint8_t>> Read(const std::string& key);
    bool Delete(const std::string& key);
    bool Update(const std::string& key, std::shared_ptr<std::vector<uint8_t>> data);
    uint32_t Create(const std::string& key, std::shared_ptr<std::vector<uint8_t>> data);
private:
    static uint32_t toInt32(const std::vector<uint8_t>& intBytes, uint32_t start)
    {
        return (intBytes[start + 3] << 24) | (intBytes[start + 2] << 16) | (intBytes[start + 1] << 8) | intBytes[start];
    }
    static uint16_t toInt16(const std::vector<uint8_t>& intBytes, uint32_t start)
    {
        return  (intBytes[start + 1] << 8) | intBytes[start];
    }

    void InternalConnect();

    std::string host_;
    uint16_t port_;
    asio::ip::tcp::socket socket_;
    asio::ip::tcp::resolver resolver_;
};

