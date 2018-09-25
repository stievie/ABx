#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_SERVICELIST = "service_list";

struct ServiceList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_SERVICELIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(uuids, Limits::MAX_SERVICES, [&s](std::string& id)
        {
            s.text1b(id, Limits::MAX_UUID);
        });
    }

    std::vector<std::string> uuids;
};

}
}
