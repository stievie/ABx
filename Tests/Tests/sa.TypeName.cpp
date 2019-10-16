#include "stdafx.h"
#include <catch.hpp>

#include <sa/TypeName.h>

class Baz
{

};

namespace Foo {
class Bar
{

};
}

TEST_CASE("TypeName no NS")
{
    constexpr auto res = sa::TypeName<Baz>::Get();
    // Hm, interesting it fails...
//    static_assert(res.compare("class Baz") == 0);
    REQUIRE(res.compare("class Baz") == 0);
}

TEST_CASE("TypeName NS")
{
    constexpr auto res = sa::TypeName<Foo::Bar>::Get();
    REQUIRE(res.compare("class Foo::Bar") == 0);
}
