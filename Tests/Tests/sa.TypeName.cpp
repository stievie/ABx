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

#include "stdafx.h"
#include <catch.hpp>

#include <sa/TypeName.h>
#include <sa/StringHash.h>
#include <string_view>
#include <sa/StrongType.h>

class Baz
{

};

namespace Foo {
class Bar
{

};
}

template <typename T>
struct TemplateStruct
{

};

TEST_CASE("TypeName no NS")
{
    constexpr auto res = sa::TypeName<Baz>::Get();
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

TEST_CASE("TypeName Hash")
{
    constexpr auto res = sa::TypeName<Foo::Bar>::Get();
    constexpr size_t hash = sa::StringHash(res);
    INFO(hash);
    REQUIRE(hash == sa::StringHashRt(res.data(), res.size()));
}

TEST_CASE("Template type")
{
    constexpr auto res = sa::TypeName<TemplateStruct<int>>::Get();
    INFO(res);
#if defined __GNUC__
    REQUIRE(res.compare("TemplateStruct<int>") == 0);
#else
    // struct TemplateStruct<int>
    REQUIRE(res.compare("struct TemplateStruct<int> ") == 0);
#endif
}