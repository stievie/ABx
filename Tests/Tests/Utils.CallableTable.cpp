#include "stdafx.h"
#include <catch.hpp>

#include "CallableTable.h"

template <typename T>
class ClassTemplateWithMedhod
{
public:
    T Foo(T i)
    {
        return i;
    }
};

class ClassWithMedhodTemplate
{
public:
    template <typename T>
    T Foo(T i)
    {
        return i;
    }
};

TEST_CASE("CallableTable")
{
    SECTION("Lambda")
    {
        Utils::CallableTable<unsigned, int, int> table;
        table.Add(1, [](int i) -> int
        {
            return i;
        });
        REQUIRE(table.Call(1, 1) == 1);

        Utils::CallableTable<unsigned, int, int, int> table2;
        table2.Add(1, [](int i, int j) -> int
        {
            return i * j;
        });
        REQUIRE(table2.Call(1, 1, 2) == 2);
    }

    SECTION("Method")
    {
        class ClassWithMedhod
        {
        public:
            int Foo(int i)
            {
                return i;
            }
        };
        ClassWithMedhod o;
        Utils::CallableTable<unsigned, int, int> table;
        table.Add(1, std::bind(&ClassWithMedhod::Foo, &o, std::placeholders::_1));
        REQUIRE(table.Call(1, 1) == 1);
    }

    SECTION("Class template method")
    {
        ClassTemplateWithMedhod<int> o;
        Utils::CallableTable<unsigned, int, int> table;
        table.Add(1, std::bind(&ClassTemplateWithMedhod<int>::Foo, &o, std::placeholders::_1));
        REQUIRE(table.Call(1, 1) == 1);
    }

    SECTION("Class method template")
    {
        ClassWithMedhodTemplate o;
        Utils::CallableTable<unsigned, int, int> table;
        table.Add(1, std::bind(&ClassWithMedhodTemplate::Foo<int>, &o, std::placeholders::_1));
        REQUIRE(table.Call(1, 1) == 1);
    }

    SECTION("Operator []")
    {
        Utils::CallableTable<unsigned, int, int, int> table;
        table.Add(1, [](int i, int j) -> int
        {
            return i * j;
        });
        REQUIRE(table[1](2, 2) == 4);
    }
}
