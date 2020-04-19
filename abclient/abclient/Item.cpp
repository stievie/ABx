/**
 * Copyright 2017-2020 Stefan Ascher
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
#include "Item.h"
#include <string>
#include <base64.h>
#include <locale>
#include <iostream>

Item::Item(Context* context) :
    Object(context)
{
}

Item::~Item()
{
}

String FormatMoney(uint32_t amount)
{
    char separator = std::use_facet<std::numpunct<char>>(std::cout.getloc()).thousands_sep();
    auto src = std::to_string(amount);
    std::string result;

    auto count = 3;
    for (auto i = src.crbegin(); i != src.crend(); ++i)
    {
        if (count == 0)
        {
            result.push_back(separator);
            count = 3;
        }
        if (count--)
            result.push_back(*i);
    }
    std::reverse(result.begin(), result.end());
    return String(result.c_str());
}
