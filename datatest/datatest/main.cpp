// datatest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include "DataClient.h"

#if 0
uint32_t toInt32(const std::vector<uint8_t>& intBytes, uint32_t start)
{
    return (intBytes[start + 3] << 24) | (intBytes[start + 2] << 16) | (intBytes[start + 1] << 8) | intBytes[start];
}

uint16_t toInt16(const std::vector<uint8_t>& intBytes, uint32_t start)
{
    return  (intBytes[start + 1] << 8) | intBytes[start];
}

void sendData(asio::ip::tcp::socket& socket, std::string& key, std::string& data)
{
    uint8_t ksize1 = key.size();
    uint8_t ksize2 = key.size() >> 8;
    uint8_t header[] = { 0,ksize1,ksize2 };
    socket.send(asio::buffer(header));



    socket.send(asio::buffer(key));

    uint8_t dsize4 = data.size() >> 24;
    uint8_t dsize3 = data.size() >> 16;
    uint8_t dsize2 = data.size() >> 8;
    uint8_t dsize1 = data.size();

    uint8_t data_header[] = { dsize1,dsize2,dsize3,dsize4 };
    socket.send(asio::buffer(data_header));


    socket.send(asio::buffer(data));

    uint8_t result[128];
    int read = socket.read_some(asio::buffer(result));
}

void deleteData(asio::ip::tcp::socket& socket, std::string& key)
{
    uint8_t ksize1 = key.size();
    uint8_t ksize2 = key.size() >> 8;
    uint8_t header[] = { 2,ksize1,ksize2 };
    socket.send(asio::buffer(header));

    socket.send(asio::buffer(key));

    uint8_t dataheader[5];
    int read = asio::read(socket, asio::buffer(dataheader, 5), asio::transfer_at_least(5));

    const int code = dataheader[4] << 24 | dataheader[3] << 16 | dataheader[2] << 8 | dataheader[1];
    int size = 2;


    std::vector<uint8_t> data(size);
    int read2 = asio::read(socket, asio::buffer(data), asio::transfer_at_least(size));
}

std::string getData(asio::ip::tcp::socket& socket, std::string& key)
{
    uint8_t ksize1 = key.size();
    uint8_t ksize2 = key.size() >> 8;
    uint8_t header[] = { 1,ksize1,ksize2 };
    socket.send(asio::buffer(header));

    socket.send(asio::buffer(key));
    std::vector<uint8_t> dataheader(5);
    int read = asio::read(socket, asio::buffer(dataheader, 5), asio::transfer_at_least(5));

    const int size = toInt32(dataheader, 1);
    //TODO handle message statuses
    std::vector<uint8_t> data(size);
    int read2 = asio::read(socket, asio::buffer(data), asio::transfer_at_least(size));

    return{ data.begin(),data.end() };
}
#endif

int main()
{
    asio::io_service io_service;
    DataClient cli(io_service);
    cli.Connect("localhost", 2770);

/*
    asio::ip::tcp::resolver resolver(io_service);
    asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), "localhost", "2770");
    asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(query);
    asio::ip::tcp::socket socket(io_service);

    asio::connect(socket, endpoint);
    std::string key = "testkey1";
    std::string data = "testdata1testdata1testdata1";
    sendData(socket, key, data);

    std::string resp = getData(socket, key);

    resp.push_back(2);
    resp.push_back('!');
    sendData(socket, key, resp);


    //deleteData(socket,key);

    std::string resp2 = getData(socket, key);

    std::cout << "Enter any key and press enter to exit" << std::endl;
    int k;
    std::cin >> k;
    socket.close();

    */
    return 0;
}

