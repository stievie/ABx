#include <catch.hpp>

#include <sa/color.h>

TEST_CASE("Color Construct")
{
    sa::color col = sa::color::from_rgb(1.0f, 1.0f, 1.0f);
    REQUIRE(col.r_ == 255);
    REQUIRE(col.g_ == 255);
    REQUIRE(col.b_ == 255);
    REQUIRE(col.a_ == 255);
}

TEST_CASE("Color from string")
{
    sa::color col = sa::color::from_string("ff00ff");
    REQUIRE(col.r_ == 255);
    REQUIRE(col.g_ == 0);
    REQUIRE(col.b_ == 255);
    REQUIRE(col.a_ == 255);

    sa::color col2 = sa::color::from_string("ff00ffaa");
    REQUIRE(col2.r_ == 255);
    REQUIRE(col2.g_ == 0);
    REQUIRE(col2.b_ == 255);
    REQUIRE(col2.a_ == 0xaa);
}

TEST_CASE("Color to string")
{
    sa::color col = sa::color::from_string("ff00ff");
    std::string str = col.to_string();
    REQUIRE(str == "ff00ffff");
}

TEST_CASE("Color to unit32_t")
{
    sa::color col = sa::color::from_rgb(1.0f, 0.0f, 0.0f, 0.0f);
    uint32_t value = static_cast<uint32_t>(col);
    REQUIRE(value == 255);
}

TEST_CASE("Color to HSL")
{
    sa::color col = sa::color::from_rgb(1.0f, 1.0f, 1.0f);
    const auto [h, s, l] = col.to_hsl();
    REQUIRE(h == Approx(0.0f));
    REQUIRE(s == Approx(0.0f));
    REQUIRE(l == Approx(1.0f));
}

TEST_CASE("Color from HSL")
{
    sa::color col = sa::color::from_hsl(0.0f, 0.0f, 1.0f);
    REQUIRE(col.r_ == 255);
    REQUIRE(col.g_ == 255);
    REQUIRE(col.b_ == 255);
    REQUIRE(col.a_ == 255);
}

TEST_CASE("Color from HSL2")
{
    sa::color col = sa::color::from_hsl(0.0f, 1.0f, 0.5f);
    REQUIRE(col.r_ == 255);
    REQUIRE(col.g_ == 0);
    REQUIRE(col.b_ == 0);
}

TEST_CASE("Color to HSV")
{
    sa::color col = sa::color::from_rgb(1.0f, 1.0f, 1.0f);
    const auto [h, s, v] = col.to_hsv();
    REQUIRE(h == Approx(0.0f));
    REQUIRE(s == Approx(0.0f));
    REQUIRE(v == Approx(1.0f));
}

TEST_CASE("Color from HSV")
{
    sa::color col = sa::color::from_hsl(0.0f, 0.0f, 1.0f);
    REQUIRE(col.r_ == 255);
    REQUIRE(col.g_ == 255);
    REQUIRE(col.b_ == 255);
    REQUIRE(col.a_ == 255);
}

TEST_CASE("Color tint")
{
    sa::color col = sa::color::from_rgb(1.0f, 0.0f, 0.0f);
    col.tint(0.5f);
    REQUIRE(col.r_ == 255);
    REQUIRE(col.g_ == 127);
    REQUIRE(col.b_ == 127);
}

TEST_CASE("Color shade")
{
    sa::color col = sa::color::from_rgb(1.0f, 0.0f, 0.0f);
    col.shade(0.5f);
    REQUIRE(col.r_ == 127);
    REQUIRE(col.g_ == 0);
    REQUIRE(col.b_ == 0);
}

TEST_CASE("Color invert")
{
    sa::color col = sa::color::from_rgb(1.0f, 0.2f, 0.0f);
    col.invert();
    REQUIRE(col.r_ == 0);
    REQUIRE(col.g_ == 204);
    REQUIRE(col.b_ == 255);
}

TEST_CASE("Color lerp")
{
    sa::color col = sa::color::from_rgb(1.0f, 0.2f, 0.0f);
    sa::color col2 = sa::color::from_rgb(0.0f, 0.2f, 1.0f);
    sa::color col3 = col.lerp(col2, 0.5f);
    REQUIRE(col3.r_ == 127);
    REQUIRE(col3.g_ == 51);
    REQUIRE(col3.b_ == 127);
}
