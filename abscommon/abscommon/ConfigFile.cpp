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

#include "ConfigFile.h"
#include <iostream>
#include <fstream>

namespace IO {

ConfigFile::ConfigFile()
{ }

ConfigFile::~ConfigFile()
{ }

bool ConfigFile::Load(const std::string& filename)
{
    entires_.clear();

    std::ifstream in(filename);
    if (!in.is_open())
        return false;

    std::string line;
    while (std::getline(in, line))
    {
        line = sa::Trim<char>(line, " \t");
        if (line.empty() || line[0] == '#')
            continue;

        auto parts = sa::Split(line, "=", false, false);
        if (parts.size() != 2)
            continue;

        entires_.emplace(sa::Trim<char>(parts[0]), sa::Trim<char>(parts[1]));
    }

    return true;
}

}
