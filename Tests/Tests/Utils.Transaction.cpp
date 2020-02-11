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

#include <sa/Transaction.h>

TEST_CASE("Transaction")
{
    struct MyType
    {
        std::string mystring;
        unsigned uint;
        std::vector<std::string> strings;
        std::vector<unsigned> uints;

        bool operator == (const MyType& other)
        {
            if (mystring.compare(other.mystring) != 0)
                return false;
            if (uint != other.uint)
                return false;
            if (strings.size() != other.strings.size())
                return false;
            size_t i = 0;
            for (const auto& s : strings)
            {
                if (s.compare(other.strings[i]) != 0)
                    return false;
                ++i;
            }
            if (uints.size() != other.uints.size())
                return false;
            size_t i2 = 0;
            for (auto u : uints)
            {
                if (u != other.uints[i2])
                    return false;
                ++i2;
            }
            return true;
        }
    };

    SECTION("Rollback")
    {
        MyType t1;
        t1.mystring = "string";
        t1.uint = 5;
        t1.strings.push_back("strings1");
        t1.strings.push_back("strings2");
        t1.uints.push_back(1);
        t1.uints.push_back(2);
        MyType t2 = t1;
        {
            sa::Transaction transaction(t1);
            t1.mystring = "nothing";
            t1.uint = 0;
            t1.strings.clear();
            t1.uints.clear();
        }
        bool equal = t1 == t2;
        REQUIRE(equal);
    }
    SECTION("Commit")
    {
        MyType t1;
        t1.mystring = "string";
        t1.uint = 5;
        t1.strings.push_back("strings1");
        t1.strings.push_back("strings2");
        t1.uints.push_back(1);
        t1.uints.push_back(2);
        MyType t2 = t1;

        sa::Transaction transaction(t1);
        t1.mystring = "nothing";
        t1.uint = 0;
        t1.strings.clear();
        t1.uints.clear();
        transaction.Commit();
        bool equal = t1 == t2;
        REQUIRE(!equal);
    }

}
