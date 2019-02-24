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
