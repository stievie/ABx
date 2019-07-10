#pragma once

#include <set>

namespace Net {

class IpList
{
private:
    std::set<uint32_t> ips_;
public:
    IpList() = default;
    ~IpList() = default;

    void Add(uint32_t ip);
    void Add(const std::string& ip);
    void AddList(const std::string& ips);
    bool Contains(uint32_t value) const;
    bool IsEmpty() const { return ips_.empty(); }
    std::string ToString() const;
};

}
