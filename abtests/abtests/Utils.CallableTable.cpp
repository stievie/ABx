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

#include <sa/CallableTable.h>

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
        sa::CallableTable<unsigned, int, int> table;
        table.Add(1, [](int i) -> int
        {
            return i;
        });
        REQUIRE(table.Call(1, 1) == 1);

        sa::CallableTable<unsigned, int, int, int> table2;
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
        sa::CallableTable<unsigned, int, int> table;
        table.Add(1, std::bind(&ClassWithMedhod::Foo, &o, std::placeholders::_1));
        REQUIRE(table.Call(1, 1) == 1);
    }

    SECTION("Class template method")
    {
        ClassTemplateWithMedhod<int> o;
        sa::CallableTable<unsigned, int, int> table;
        table.Add(1, std::bind(&ClassTemplateWithMedhod<int>::Foo, &o, std::placeholders::_1));
        REQUIRE(table.Call(1, 1) == 1);
    }

    SECTION("Class method template")
    {
        ClassWithMedhodTemplate o;
        sa::CallableTable<unsigned, int, int> table;
        table.Add(1, std::bind(&ClassWithMedhodTemplate::Foo<int>, &o, std::placeholders::_1));
        REQUIRE(table.Call(1, 1) == 1);
    }

    SECTION("Operator []")
    {
        sa::CallableTable<unsigned, int, int, int> table;
        table.Add(1, [](int i, int j) -> int
        {
            return i * j;
        });
        REQUIRE(table[1](2, 2) == 4);
    }
}
