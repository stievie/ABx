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
