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

#pragma once

#include <algorithm>

namespace Utils {
namespace XML {

template<typename InIter, typename OutIter>
OutIter CopyAsciiz(InIter begin, OutIter out)
{
    while (*begin != '\0')
        *out++ = *begin++;
    return (out);
}

// XML escaping in it's general form.  Note that 'out' is expected
// to an "infinite" sequence.
template<typename InIter, typename OutIter>
OutIter Escape(InIter begin, InIter end, OutIter out)
{
    constexpr char bad[] = "&<>";
    constexpr std::size_t n = sizeof(bad) / sizeof(bad[0]);
    static const char* rep[] = { "&amp;", "&lt;", "&gt;" };

    for (; (begin != end); ++begin)
    {
        // Find which replacement to use.
        const std::size_t i =
            std::distance(bad, std::find(bad, bad + n, *begin));

        // No need for escaping.
        if (i == n)
            *out++ = *begin;
        // Escape the character.
        else
            out = CopyAsciiz(rep[i], out);
    }
    return (out);
}

// Get escaped version of "content".
std::string Escape(const std::string& content);
// Escape data on the fly, using "constant" memory.
void Escape(std::istream& in, std::ostream& out);

}
}
