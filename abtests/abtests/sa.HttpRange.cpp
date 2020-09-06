#include <catch.hpp>

#include <sa/http_range.h>

TEST_CASE("HTTP Range parse basic")
{
    sa::http::ranges ranges;
    bool success = sa::http::parse_ranges(1024, "bytes=200-1000", ranges);
    REQUIRE(success);
    REQUIRE(ranges.size() == 1);
}

TEST_CASE("HTTP Range parse 3 ranges suffix")
{
    sa::http::ranges ranges;
    bool success = sa::http::parse_ranges(20000000, "bytes=200-1000, 2000-6576, 19000-", ranges);
    REQUIRE(success);
    REQUIRE(ranges.size() == 3);
}

TEST_CASE("HTTP Range parse 2 ranges suffix")
{
    sa::http::ranges ranges;
    bool success = sa::http::parse_ranges(20000000, "bytes=0-499, -500", ranges);
    REQUIRE(success);
    REQUIRE(ranges.size() == 2);
    auto& r1 = ranges[0];
    REQUIRE(r1.start == 0);
    REQUIRE(r1.end == 499);
    auto& r2 = ranges[1];
    REQUIRE(r2.start == 20000000 - 500);
    REQUIRE(r2.end == 20000000);
}

TEST_CASE("HTTP Range parse overlap")
{
    sa::http::ranges ranges;
    bool success = sa::http::parse_ranges(20000000, "bytes=200-1000, 2000-6576, 2500-9000, 500-1500", ranges);
    REQUIRE(success);
    REQUIRE(ranges.size() == 2);
    auto& r1 = ranges[0];
    REQUIRE(r1.start == 200);
    REQUIRE(r1.end == 1500);
    auto& r2 = ranges[1];
    REQUIRE(r2.start == 2000);
    REQUIRE(r2.end == 9000);
}
