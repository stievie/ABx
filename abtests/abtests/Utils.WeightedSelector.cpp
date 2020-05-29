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

#include <sa/WeightedSelector.h>

TEST_CASE("WeightedSelector")
{
    sa::WeightedSelector<std::string> ws;
    ws.Add("Test1", 0.5f);
    ws.Add("Test2", 0.2f);
    ws.Add("Test3", 0.15f);
    ws.Add("Test4", 0.1f);
    ws.Add("Test5", 0.05f);
    ws.Add("Test6", 0.05f);
    ws.Add("Test7", 0.05f);
    ws.Add("Test8", 0.05f);
    ws.Add("Test9", 0.05f);
    ws.Add("Test10", 0.05f);
    ws.Update();

    std::string s = ws.Get(0.3f, 0.25f);
    REQUIRE(s == "Test4");
}
