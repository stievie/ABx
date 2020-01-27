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
    template<typename T>
    void value_enc(T& value)
    {
        value = msg_.template GetDecrypted<T>();
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
    template<typename T>
    void value_enc(T& value)
    {
        msg_.template AddEncryted<T>(value);
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
