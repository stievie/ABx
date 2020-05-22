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


#include "ChatFilter.h"
#include "Options.h"
#include <fstream>
#include <sa/StringTempl.h>

ChatFilter::ChatFilter(Context* context) :
    Object(context)
{ }

ChatFilter::~ChatFilter()
{ }

void ChatFilter::Load()
{
    auto* options = GetSubsystem<Options>();
    String filename = AddTrailingSlash(options->GetPrefPath()) + "chat_filter.txt";
    std::ifstream file(filename.CString());
    if (!file.is_open())
        return;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        String pattern;
        pattern.AppendWithFormat("*%s*", line.c_str());
        filterPatterns_.Push(std::move(pattern));
    }
}

bool ChatFilter::Matches(const String& value)
{
    if (filterPatterns_.Size() == 0)
        return false;

    const std::string str = "_" + std::string(value.CString()) + "_";
    for (const auto& pattern : filterPatterns_)
    {
        if (sa::PatternMatch(str, std::string(pattern.CString())))
            return true;
    }
    return false;
}
