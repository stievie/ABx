#include <catch.hpp>

#include <sa/hash.h>
#include <sstream>
#include <string_view>

TEST_CASE("sa::hash compare equal")
{
    sa::hash<unsigned char, 20> h1("1dd3655c8c9cf09635b809c128965b5f79321d50");
    sa::hash<unsigned char, 20> h2("1dd3655c8c9cf09635b809c128965b5f79321d50");
    REQUIRE(h1 == h2);
}

TEST_CASE("sa::hash compare unequal")
{
    sa::hash<unsigned char, 20> h1("1dd3655c8c9cf09635b809c128965b5f79321d50");
    sa::hash<unsigned char, 20> h2("1dd3655c8c9cf09635b009c128965b5f79321d50");
    REQUIRE(h1 != h2);
}

TEST_CASE("sa::hash to string")
{
    std::string_view src("1dd3655c8c9cf09635b809c128965b5f79321d50");
    sa::hash<unsigned char, 20> h1(src);
    std::stringstream ss;
    ss << h1;
    REQUIRE(src.compare(ss.str().c_str()) == 0);
}
