#include "stdafx.h"
#include <catch.hpp>

#include "MessageMsg.h"

TEST_CASE("MessageMsg Copy")
{
    Net::MessageMsg msg1;
    msg1.type_ = Net::MessageType::Last;
    IO::PropWriteStream stream;
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
