#include "stdafx.h"
#include "Xml.h"

namespace Utils {
namespace XML {

std::string Escape(const std::string& content)
{
    std::string result;
    result.reserve(content.size());
    Escape(content.begin(), content.end(), std::back_inserter(result));
    return (result);
}

void Escape(std::istream& in, std::ostream& out)
{
    Escape(std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>(),
        std::ostreambuf_iterator<char>(out));
}

}
}
