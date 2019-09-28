#include "stdafx.h"
#include <catch.hpp>
#include <sa/SharedPtr.h>

class A
{
    char data[2048];
};

class B
{
public:
    char data{ '\0' };
};

TEST_CASE("SharedPtr")
{
    SECTION("MakeShared")
    {
        sa::SharedPtr<A> a = sa::MakeShared<A>();
        REQUIRE(a.Refs() == 1);
    }
    SECTION("Copy")
    {
        sa::SharedPtr<A> a = sa::MakeShared<A>();
        REQUIRE(a.Refs() == 1);
        sa::SharedPtr<A> a2 = a;
        REQUIRE(a.Refs() == 2);
        REQUIRE(a2.Refs() == 2);
    }
    SECTION("Move")
    {
        sa::SharedPtr<A> a = sa::MakeShared<A>();
        REQUIRE(a.Refs() == 1);
        sa::SharedPtr<A> a2 = std::move(a);
        REQUIRE(a.Refs() == 0);
        REQUIRE(a2.Refs() == 1);
    }
    SECTION("Custom make/delete")
    {
        sa::SharedPtr<B> b = sa::MakeShared<B>();
        REQUIRE(b.Refs() == 1);
    }

}
