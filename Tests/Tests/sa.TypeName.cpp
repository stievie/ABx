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
    INFO(res);
#if defined __GNUC__
    // no class or whatever
    REQUIRE(res.compare("Baz") == 0);
#else
    // MSVC
    REQUIRE(res.compare("class Baz") == 0);
#endif
}

TEST_CASE("TypeName NS")
{
    constexpr auto res = sa::TypeName<Foo::Bar>::Get();
    INFO(res);
#if defined __GNUC__
    REQUIRE(res.compare("Foo::Bar") == 0);
#else
    REQUIRE(res.compare("class Foo::Bar") == 0);
#endif
}
