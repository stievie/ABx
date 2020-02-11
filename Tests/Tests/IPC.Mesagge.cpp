/**
 * Copyright 2020 Stefan Ascher
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
#include <catch.hpp>

#include "Message.h"
#include "MessageBuffer.h"

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

TEST_CASE("IPC Message serialize")
{
    IPC::MessageBuffer buffer;
    MyMessage msg { 42, "Hello world" };
    IPC::Add(msg, buffer);

    MyMessage msg2 = IPC::Get<MyMessage>(buffer);
    REQUIRE(msg.intValue == msg2.intValue);
    REQUIRE(msg.strValue == msg2.strValue);
}
