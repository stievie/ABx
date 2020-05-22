/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <catch.hpp>
#include <sa/SmartPtr.h>

class A
{
public:
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
