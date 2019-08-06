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

TEST_CASE("MessageMsg PropStream")
{
    Net::MessageMsg msg1;
    msg1.type_ = Net::MessageType::Last;
    IO::PropWriteStream stream;
    stream.WriteString("Stuff");
    stream.Write<unsigned>(10);
    REQUIRE(msg1.SetPropStream(stream));

    Net::MessageMsg msg2 = msg1;
    msg1.Empty();
    IO::PropReadStream readStream;
    REQUIRE(msg2.GetPropStream(readStream));
    std::string str;
    REQUIRE(readStream.ReadString(str));
    REQUIRE(str.compare("Stuff") == 0);
    unsigned u;
    REQUIRE(readStream.Read<unsigned>(u));
    REQUIRE(u == 10);
}
