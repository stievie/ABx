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
#include <abscommon/StringUtils.h>

TEST_CASE("UTF-8 lowercase")
{
    std::string s1 = "ÖÄÜ";
    std::string s1lower = Utils::Utf8ToLower(s1);
    REQUIRE(s1lower.compare("öäü") == 0);
}

TEST_CASE("UTF-8 lowercase 2")
{
    std::string s1 = "Άρθουρ Ντεντ";
    std::string s1lower = Utils::Utf8ToLower(s1);
    REQUIRE(s1lower.compare("άρθουρ ντεντ") == 0);
}

TEST_CASE("UTF-8 ci equls")
{
    REQUIRE(Utils::SameName("öäü", "ÖäÜ"));
}

TEST_CASE("UTF-8 ci equls 2")
{
    REQUIRE(Utils::SameName("Άρθουρ Ντεντ", "άρθουρ ντεντ"));
}

TEST_CASE("Wide to UTF-8")
{
    std::wstring wstr = L"Άρθουρ Ντεντ";
    std::string utf8 = Utils::WStringToUtf8(wstr);
    REQUIRE(utf8.compare("Άρθουρ Ντεντ") == 0);
}

TEST_CASE("UTF-8 to Wide")
{
    std::string utf8 = "Άρθουρ Ντεντ";
    std::wstring wstr = Utils::Utf8ToWString(utf8);
    REQUIRE(wstr.compare(L"Άρθουρ Ντεντ") == 0);
}
