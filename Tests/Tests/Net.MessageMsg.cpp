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

#include <abscommon/MessageMsg.h>

TEST_CASE("MessageMsg Copy")
{
    Net::MessageMsg msg1;
    msg1.type_ = Net::MessageType::Last;
    sa::PropWriteStream stream;
    stream.WriteString("Stuff");
    stream.Write<unsigned>(10);
    msg1.SetPropStream(stream);

    SECTION("Constructor")
    {
        Net::MessageMsg msg2(msg1);
        auto result = memcmp(msg1.Data(), msg2.Data(), msg1.Length());
        REQUIRE(result == 0);
    }
    SECTION("Assignment")
    {
        Net::MessageMsg msg2 = msg1;
        auto result = memcmp(msg1.Data(), msg2.Data(), msg1.Length());
        REQUIRE(result == 0);
    }

}

TEST_CASE("MessageMsg PropStream")
{
    Net::MessageMsg msg1;
    msg1.type_ = Net::MessageType::Last;
    sa::PropWriteStream stream;
    stream.WriteString("Stuff");
    stream.Write<unsigned>(10);
    REQUIRE(msg1.SetPropStream(stream));

    Net::MessageMsg msg2 = msg1;
    msg1.Empty();
    sa::PropReadStream readStream;
    REQUIRE(msg2.GetPropStream(readStream));
    std::string str;
    REQUIRE(readStream.ReadString(str));
    REQUIRE(str.compare("Stuff") == 0);
    unsigned u;
    REQUIRE(readStream.Read<unsigned>(u));
    REQUIRE(u == 10);
}
