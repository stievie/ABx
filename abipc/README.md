# abipc

Simple IPC Server and Client.

## Start server

~~~cpp
asio::io_service io;
asio::ip::tcp::endpoint endpoint(asio::ip::address(asio::ip::address_v4(ip)), 1234);
IPC::Server server(io, endpoint);
io.run();
~~~

## Connecting to server

~~~cpp
asio::io_service io;
IPC::Client client(io);
client.Connect("localhost", 1234);
io.run();
~~~

## Defining messages

~~~cpp
struct MyMessage : public Message<MyMessage>
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

## Sending messages

~~~cpp
MyMessage msg { 42, "Hello friends" };
// Or server
client_.Send(msg);
~~~

## Handling messages

~~~cpp
handlers_.Add<MyMessage>([](const MyMessage& msg)
{
    std::cout << "intValue = " << msg.intValue << std::endl;
    std::cout << "strValue = " << msg.strValue << std::endl;
});
~~~
