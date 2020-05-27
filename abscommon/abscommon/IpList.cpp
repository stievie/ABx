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


#include "IpList.h"
#include "StringUtils.h"
#include <sa/StringTempl.h>

namespace Net {

void IpList::Add(uint32_t ip)
{
    ips_.emplace(ip);
}

void IpList::Add(const std::string& ip)
{
    Add(Utils::ConvertStringToIP(ip));
}

void IpList::AddList(const std::string& ips)
{
    if (ips.empty())
        return;
    const std::vector<std::string> ipVec = sa::Split(ips, ";");
    for (const std::string& ip : ipVec)
        Add(ip);
}

bool IpList::Contains(uint32_t value) const
{
    return ips_.find(value) != ips_.end();
}

std::string IpList::ToString() const
{
    std::string result;
    for (uint32_t ip : ips_)
    {
        result += Utils::ConvertIPToString(ip) + " ";
    }
    return result;
}

}
