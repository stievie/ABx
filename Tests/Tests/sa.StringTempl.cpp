#include <catch.hpp>

#include <sa/StringTempl.h>

TEST_CASE("Split")
{
    std::string input = "this is a string";
    auto parts = sa::Split(input, " ");
    REQUIRE(parts.size() == 4);
}

TEST_CASE("Split no empty")
{
    std::string input = "this|is|a|string||with|space";
    auto parts = sa::Split(input, "|");
    REQUIRE(parts.size() == 6);
}

TEST_CASE("Split with empty")
{
    std::string input = "this|is|a|string||with|space";
    auto parts = sa::Split(input, "|", true);
    REQUIRE(parts.size() == 7);
}

TEST_CASE("Split no white space")
{
    std::string input = "this|is|a|string|      |\t|with|space";
    auto parts = sa::Split(input, "|", false, false);
    REQUIRE(parts.size() == 6);
}

TEST_CASE("Trim")
{
    std::string input = "   \t string   ";
    auto trimmed = sa::Trim(input);
    REQUIRE(trimmed == "string");
}

TEST_CASE("Left Trim")
{
    std::string input = "   \t string   ";
    auto trimmed = sa::LeftTrim(input);
    REQUIRE(trimmed == "string   ");
}

TEST_CASE("Right Trim")
{
    std::string input = "   \t string   ";
    auto trimmed = sa::RightTrim(input);
    REQUIRE(trimmed.length() == std::string("   \t string").length());
    REQUIRE(trimmed == "   \t string");
}

TEST_CASE("Replace")
{
    std::string input = "this is a string";
    REQUIRE(sa::ReplaceSubstring(input, std::string(" is "), std::string(" is not ")));
    REQUIRE(input == "this is not a string");
}
