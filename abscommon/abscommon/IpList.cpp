#include "stdafx.h"
#include "IpList.h"
#include "StringUtils.h"

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
    const std::vector<std::string> ipVec = Utils::Split(ips, ";");
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
