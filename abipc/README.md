# abipc

Simple IPC Server and Client using TCP sockets.

## Characteristics

* It's really simple, just some lines of code.
* It can send any `struct` from client to server and vice versa.
* It doesn't use inheritance, polymorphism, RTTI or macros, just some templates.
* Minimal runtime overhad.
* Type safety.
* Requires C++17. Tested with GCC 8, GCC 9, MSVC 14.24 (VS 2019).
* *Not* endian safe (it mostly uses `memcpy()`).

## Dependencies

* asio https://think-async.com/Asio/AsioStandalone
* [Some headers](../Include/sa)

## Start server

~~~cpp
#include "IpcServer.h"

asio::io_service io;
asio::ip::tcp::endpoint endpoint(asio::ip::address(asio::ip::address_v4(ip)), 1234);
IPC::Server server(io, endpoint);
io.run();
~~~

## Connecting to server

~~~cpp
#include "IpcClient.h"

asio::io_service io;
IPC::Client client(io);
client.Connect("localhost", 1234);
io.run();
// Or io.poll() or whatever fits the application.
~~~

## Messages

Client and server communicate with so called "Messages". These can be
`struct`s or `class`es which must satisfy some requirements.

All messages to be sent must have a public `Serialize()` function template.
For each member of the Message `ar.value()` must be called. That's all
to make a `struct` useable by the IPC server and client.

~~~cpp
struct MyMessage
{
    int intValue;
    std::string strValue;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(intValue);
        ar.value(strValue);
    }
};
~~~

### Variable sized container

~~~cpp
struct MyMessage2
{
    // Count of members elements
    uint8_t count;
    std::vector<uint32_t> members;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(count);
        members.resize(count);
        for (uint8_t i = 0; i < count; ++i)
        {
            auto& member = members[i];
            ar.value(member);
        }
    }
};
~~~

### Variable sized container with UDT

~~~cpp
struct MyMessage3
{
    struct Item
    {
        uint16_t type;
        uint32_t index;
        uint8_t place;
        std::string name;
    };

    uint16_t count;
    std::vector<Item> items;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(count);
        items.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& item = items[i];
            ar.value(item.type);
            ar.value(item.index);
            ar.value(item.place);
            ar.value(item.name);
        }
    }
};
~~~

## Sending messages

Client to server:

~~~cpp
MyMessage msg { 42, "Hello friends" };
// Client
client_.Send(msg);
~~~

Server to client:

~~~cpp
MyMessage msg { 42, "Hello friends" };
// Server sends the message to all connected clients
server.Send(msg);
// Server sends the message to a specific client
server.SendTo(client, msg);
~~~

## Handling messages

Client and server have a `handlers_` member.

~~~cpp
// Client
handlers_.Add<MyMessage>([](const MyMessage& msg)
{
    std::cout << "intValue = " << msg.intValue << std::endl;
    std::cout << "strValue = " << msg.strValue << std::endl;
});
~~~

~~~cpp
// Server
handlers_.Add<MyMessage>([](IPC::ServerConnection& client, const MyMessage& msg)
{
    std::cout << "intValue = " << msg.intValue << std::endl;
    std::cout << "strValue = " << msg.strValue << std::endl;
});
~~~

## Limitations

This whole thing rely on the assumption that [`sa::TypeName<MyMessage>::Get()`](../Include/sa/TypeMame.h)
return exactly the same value for the server and the client. If you compile the server and
client with different compilers, make sure this works. The current implementation
of [`TypeMame.h`)](../Include/sa/TypeMame.h) works with GCC and MSVC.
