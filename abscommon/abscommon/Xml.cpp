#include "stdafx.h"
#include "Xml.h"

std::string Utils::XML::Escape(const std::string& content)
{
    std::string result;
    result.reserve(content.size());
    Escape(content.begin(), content.end(), std::back_inserter(result));
    return (result);
}

void Utils::XML::Escape(std::istream& in, std::ostream& out)
{
    Escape(std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>(),
        std::ostreambuf_iterator<char>(out));
}
