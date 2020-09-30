#include <catch.hpp>

#include <sa/path.h>
#include <string>

TEST_CASE("sa::path construct")
{
    sa::path path("/test/path");
    REQUIRE(path.string() == "/test/path");
}

TEST_CASE("sa::path compare")
{
    sa::path path1("/test/path");
    sa::path path2("/test/path");
    REQUIRE(path1 == path2);
    sa::path path3("/test/path/file");
    REQUIRE(path1 != path3);
}

TEST_CASE("sa::path concat")
{
    sa::path path1("/test/path");
    sa::path path2 = path1 / sa::path("file");
    sa::path path3("/test/path/file");
    REQUIRE(path2 == path3);
}

TEST_CASE("sa::path directory")
{
    sa::path path1("/test/path/file");
    REQUIRE(path1.directory() == "/test/path");
    path1.set_directory("/test/path2");
    REQUIRE(path1.string() == "/test/path2/file");
}

TEST_CASE("sa::path filename")
{
    sa::path path1("/test/path/file");
    REQUIRE(path1.filename() == "file");
    path1.set_filename("file2");
    REQUIRE(path1.string() == "/test/path/file2");
}

TEST_CASE("sa::path ext")
{
    sa::path path1("/test/path/file.ext");
    REQUIRE(path1.ext() == ".ext");
    path1.set_ext(".ext2");
    REQUIRE(path1.string() == "/test/path/file.ext2");
}
