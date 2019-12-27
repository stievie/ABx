#pragma once

namespace AB {
namespace Packets {

namespace detail {

template <typename _Msg>
class Reader
{
private:
    _Msg& msg_;
public:
    Reader(_Msg& msg) :
        msg_(msg)
    { }
    template<typename T>
    void value(T& value)
    {
        value = msg_.template Get<T>();
    }
};

template <typename _Msg>
class Writer
{
private:
    _Msg& msg_;
public:
    Writer(_Msg& msg) :
        msg_(msg)
    { }
    template<typename T>
    void value(T& value)
    {
        msg_.template Add<T>(value);
    }
};

}

// Read a packet from the message
template<typename T, typename _Msg>
T Get(_Msg& msg)
{
    T result;
    detail::Reader<_Msg> reader(msg);
    result.Serialize(reader);
    return result;
}

// Add a packet to the message
template<typename T, typename _Msg>
void Add(T& value, _Msg& msg)
{
    // Can not be const T& value because we are using the same function for reading
    // and writing which is not const
    detail::Writer<_Msg> writer(msg);
    value.Serialize(writer);
}

}
}
