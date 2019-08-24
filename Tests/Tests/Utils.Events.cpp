#include "stdafx.h"
#include <catch.hpp>
#include <functional>

#include "Events.h"

TEST_CASE("Events")
{
    class Foo
    {
    private:
        Utils::Events<
            int(int, int)                 // Lambda
        > events;
        int Bar(int i, int j)
        {
            return i * j;
        }
    public:
        Foo()
        {
            events.Add<int(int, int)>(1, std::bind(&Foo::Bar, this, std::placeholders::_1, std::placeholders::_2));
        }
        int DoBar(int i, int j)
        {
            return events.Call<int(int, int)>(1, i, j);
        }
    };

    SECTION("Lambda")
    {
        Utils::Events<
            int(int, int)                 // Lambda
        > events;
        events.Add<int(int, int)>(1, [](int i, int j) -> int
        {
            return i * j;
        });
        auto result = events.Call<int(int, int)>(1, 2, 3);
        REQUIRE(result == 6);

        auto func = [](int i, int j) -> int
        {
            return i + j;
        };
        events.Add<int(int, int)>(2, func);
        auto result2 = events.Call<int(int, int)>(2, 4, 5);
        REQUIRE(result2 == 9);
    }

    SECTION("Not found")
    {
        Utils::Events<
            int(int, int)
        > events;
        events.Add<int(int, int)>(1, [](int i, int j) -> int
        {
            return i * j;
        });
        auto result = events.Call<int(int, int)>(2, 2, 3);
        REQUIRE(result == 0);
    }

    SECTION("Method")
    {
        Foo foo;
        auto result = foo.DoBar(3, 2);
        REQUIRE(result == 6);
    }
}