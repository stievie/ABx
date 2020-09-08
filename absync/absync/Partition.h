/**
 * Copyright (c) 2018 Rokas Kupstys
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

#pragma once

#include <vector>
#include <string>
#include <sa/Compiler.h>
#if defined(SA_MSVC)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#elif defined(SA_PLATFORM_LINUX)
#include <sys/types.h>
#endif
#include "Parameters.h"
#include <string_view>

namespace Sync {

static constexpr const char* META_FILE_EXT = ".meta";

struct Boundary
{
    size_t start{ 0 };
    uint32_t fingerprint{ 0 };
    size_t hash{ 0 };
    size_t length{ 0 };
};

using BoundaryList = std::vector<Boundary>;

BoundaryList PartitionFile(const std::string& filename, const Parameters& params);
void SaveBoundaryList(const std::string& filename, const BoundaryList& list);
BoundaryList LoadBoundaryList(const std::string& filename);
BoundaryList LoadBoundaryList(const std::vector<char>& buffer);
BoundaryList LoadBoundaryList(std::string_view buffer);

}
