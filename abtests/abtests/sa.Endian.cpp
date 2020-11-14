#include <catch.hpp>

#include <sa/Endian.h>

TEST_CASE("Little Endian")
{
    uint32_t value = 42;
    uint32_t littleValue = sa::ConvertLittleEndian(value);
#if defined(SA_LITTLE_ENDIAN)
    REQUIRE(littleValue == value);
#else
    REQUIRE(littleValue == 704643072);
#endif
    uint32_t origValue = sa::ConvertLittleEndian(littleValue);
    REQUIRE(value == origValue);
}

TEST_CASE("Big Endian")
{
    uint32_t value = 42;
    uint32_t bigValue = sa::ConvertBigEndian(value);
#if defined(SA_BIG_ENDIAN)
    REQUIRE(bigValue == value);
#else
    REQUIRE(bigValue == 704643072);
#endif
    uint32_t origValue = sa::ConvertBigEndian(bigValue);
    REQUIRE(value == origValue);
}

TEST_CASE("Little Endian float")
{
    float value = 42.0f;
    float littleValue = sa::ConvertLittleEndian(value);
    float origValue = sa::ConvertLittleEndian(littleValue);
    REQUIRE(value == origValue);
}

TEST_CASE("Big Endian float")
{
    float value = 42.0f;
    float bigValue = sa::ConvertBigEndian(value);
    float origValue = sa::ConvertBigEndian(bigValue);
    REQUIRE(value == origValue);
}
