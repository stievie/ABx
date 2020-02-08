# abipc

Simple IPC Server and Client.

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
