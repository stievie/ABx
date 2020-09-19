#include <catch.hpp>

#define SA_ZLIB_SUPPORT
#define SA_LZ4_SUPPORT
#include <sa/compress.h>
#include <string>

TEST_CASE("sa::compress zlib")
{
    sa::zlib_compress compress(false);
    std::string input = "is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic";
    std::string output;
    output.resize(input.length());
    size_t outputSize = output.length();
    compress(input.c_str(), input.length(), output.data(), outputSize);

    sa::zlib_decompress decompress(false);
    std::string decompressed;
    decompressed.resize(outputSize * 2);
    size_t decompressedSize = decompressed.length();
    decompress(output.c_str(), outputSize, decompressed.data(), decompressedSize);
    decompressed.resize(decompressedSize);
    REQUIRE(input.compare(decompressed) == 0);
    float ratio = (float)outputSize / (float)input.length();
    (void)ratio;
}

TEST_CASE("sa::compress zlib gzip")
{
    sa::zlib_compress compress;
    std::string input = "is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic";
    std::string output;
    output.resize(input.length());
    size_t outputSize = output.length();
    compress(input.c_str(), input.length(), output.data(), outputSize);

    sa::zlib_decompress decompress;
    std::string decompressed;
    decompressed.resize(outputSize * 2);
    size_t decompressedSize = decompressed.length();
    decompress(output.c_str(), outputSize, decompressed.data(), decompressedSize);
    decompressed.resize(decompressedSize);
    REQUIRE(input.compare(decompressed) == 0);
    float ratio = (float)outputSize / (float)input.length();
    (void)ratio;
}

TEST_CASE("sa::compress lz4")
{
    sa::lz4_compress compress;
    std::string input = "is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic";
    std::string output;
    output.resize(input.length());
    size_t outputSize = output.length();
    compress(input.data(), input.length(), output.data(), outputSize);

    sa::lz4_decompress decompress;
    std::string decompressed;
    decompressed.resize(outputSize * 2);
    size_t decompressedSize = decompressed.length();
    decompress(output.data(), outputSize, decompressed.data(), decompressedSize);
    decompressed.resize(decompressedSize);
    REQUIRE(input.compare(decompressed) == 0);
    float ratio = (float)outputSize / (float)input.length();
    (void)ratio;
}
