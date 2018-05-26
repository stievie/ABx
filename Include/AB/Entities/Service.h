#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

enum ServiceType : uint8_t
{
    ServiceTypeUnknown = 0,
    ServiceTypeDataServer,
    ServiceTypeFileServer,
    ServiceTypeGameServer
};

enum ServiceStatus : uint8_t
{
    ServiceStatusOffline = 0,
    ServiceStatusOnline
};

constexpr auto KEY_SERVICES = "services";

struct Service : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_SERVICES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_SERVICE_NAME);
        s.value1b(type);
        s.text1b(host, Limits::MAX_SERVICE_HOST);
        s.value1b(status);
        s.value8b(startTime);
        s.value8b(stopTime);
        s.value8b(runTime);
    }

    std::string name;
    ServiceType type = ServiceTypeUnknown;
    std::string host;
    ServiceStatus status = ServiceStatusOffline;
    int64_t startTime = 0;
    int64_t stopTime = 0;
    int64_t runTime = 0;
};

}
}
