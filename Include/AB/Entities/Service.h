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
    ServiceTypeLoginServer,
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
        s.value2b(port);
        s.value2b(statusPort);
        s.value1b(status);
        s.value8b(startTime);
        s.value8b(stopTime);
        s.value8b(runTime);
        s.text1b(file, Limits::MAX_FILENAME);
        s.text1b(path, Limits::MAX_FILENAME);
        s.text1b(arguments, Limits::MAX_FILENAME);
        s.value1b(load);
    }

    std::string name;
    ServiceType type = ServiceTypeUnknown;
    std::string host;
    uint16_t port = 0;
    uint16_t statusPort = 0;
    ServiceStatus status = ServiceStatusOffline;
    int64_t startTime = 0;
    int64_t stopTime = 0;
    int64_t runTime = 0;
    std::string file;
    std::string path;
    std::string arguments;

    /// Service load, something between 0..100. Not written to DB. The service
    /// is responsible to update this value.
    uint8_t load = 0;
};

}
}
