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
#include <functional>

#include <sa/Events.h>

TEST_CASE("Events")
{
    class Foo
    {
    private:
        sa::Events<
            int(int, int)
        > events;
        int Bar(int i, int j)
        {
            return i * j;
        }
    public:
        Foo()
        {
            events.Subscribe<int(int, int)>(1, std::bind(&Foo::Bar, this, std::placeholders::_1, std::placeholders::_2));
        }
        int DoBar(int i, int j)
        {
            return events.CallOne<int(int, int)>(1, i, j);
        }
    };

    class A
    {
    private:
        sa::Events<
            int(int, int)
        > events;
    protected:
        virtual int Foo(int i, int j)
        {
            return i * j;
        }
    public:
        A()
        {
            events.Subscribe<int(int, int)>(1, std::bind(&A::Foo, this, std::placeholders::_1, std::placeholders::_2));
        }
        virtual ~A() = default;
        int DoFoo(int i, int j)
        {
            return events.CallOne<int(int, int)>(1, i, j);
        }
    };
    class B : public A
    {
    protected:
        int Foo(int i, int j) override
        {
            return i + j;
        }
    };


    SECTION("Lambda")
    {
        sa::Events<
            int(int, int)
        > events;
        events.Subscribe<int(int, int)>(1, [](int i, int j) -> int
        {
            return i * j;
        });
        auto result = events.CallOne<int(int, int)>(1, 2, 3);
        static_assert(std::is_same<decltype(result), int>::value);
        REQUIRE(result == 6);

        auto func = [](int i, int j) -> int
        {
            return i + j;
        };
        events.Subscribe<int(int, int)>(2, func);
        auto result2 = events.CallOne<int(int, int)>(2, 4, 5);
        static_assert(std::is_same<decltype(result2), int>::value);
        REQUIRE(result2 == 9);
    }

    SECTION("Not found")
    {
        sa::Events<
            int(int, int)
        > events;
        events.Subscribe<int(int, int)>(1, [](int i, int j) -> int
        {
            return i * j;
        });
        auto result = events.CallOne<int(int, int)>(2, 2, 3);
        REQUIRE(result == 0);
    }

    SECTION("Method")
    {
        Foo foo;
        auto result = foo.DoBar(3, 2);
        REQUIRE(result == 6);
    }

    SECTION("Virtual Method")
    {
        B b;
        auto result = b.DoFoo(3, 2);
        // Must be 5 because B overrides it does not multiply it but calculates the sum
        REQUIRE(result == 5);
    }

    SECTION("Different signatures")
    {
        sa::Events<
            int(int, int),
            bool(int),
            void(void)
        > events;
        events.Subscribe<int(int, int)>(1, [](int i, int j) -> int
        {
            return i * j;
        });
        events.Subscribe<bool(int)>(2, [](int i) -> bool
        {
            return i != 0;
        });

        auto result = events.CallOne<int(int, int)>(1, 4, 5);
        static_assert(std::is_same<decltype(result), int>::value);
        REQUIRE(result == 20);

        auto result2 = events.CallOne<bool(int)>(2, 5);
        static_assert(std::is_same<decltype(result2), bool>::value);
        REQUIRE(result2 == true);
    }

    SECTION("Multiple subscribers")
    {
        sa::Events<
            std::string(const std::string&)
        > events;
        events.Subscribe<std::string(const std::string&)>(1, [](const std::string& s)
        {
            return s;
        });
        events.Subscribe<std::string(const std::string&)>(1, [](const std::string& s)
        {
            return s;
        });

        auto results = events.CallAll<std::string(const std::string&)>(1, "Hello Subscribers!");
        REQUIRE(results.size() == 2);
        for (const auto& s : results)
            REQUIRE(s.compare("Hello Subscribers!") == 0);
    }

    SECTION("Event not found")
    {
        sa::Events<
            int(int, int)
        > events;
        events.Subscribe<int(int, int)>(1, [](int i, int j) -> int
        {
            return i * j;
        });
        auto result = events.CallOne<int(int, int)>(2, 2, 3);
        static_assert(std::is_same<decltype(result), int>::value);
        REQUIRE(result == 0);
    }
}
