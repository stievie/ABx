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

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

enum ServiceType : uint8_t
{
    ServiceTypeUnknown = 0,
    ServiceTypeDataServer,
    ServiceTypeMessageServer,
    ServiceTypeFileServer,
    ServiceTypeLoginServer,
    ServiceTypeGameServer,
    ServiceTypeMatchServer,

    ServiceTypeAdminServer,

    // Must be last
    ServiceTypeLoadBalancer = 255
};

enum ServiceStatus : uint8_t
{
    ServiceStatusOffline = 0,
    ServiceStatusOnline
};

inline constexpr unsigned HEARTBEAT_INTERVAL = 1000;

struct Service : Entity
{
    MAKE_ENTITY(Service)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_SERVICE_NAME);
        s.value4b(version);
        s.value1b(type);
        s.text1b(location, Limits::MAX_SERVICE_LOCATION);
        s.text1b(host, Limits::MAX_SERVICE_HOST);
        s.value2b(port);
        s.text1b(ip, Limits::MAX_SERVICE_IP);
        s.value1b(status);
        s.value8b(startTime);
        s.value8b(stopTime);
        s.value8b(runTime);
        s.text1b(machine, Limits::MAX_FILENAME);
        s.text1b(file, Limits::MAX_FILENAME);
        s.text1b(path, Limits::MAX_FILENAME);
        s.text1b(arguments, Limits::MAX_FILENAME);
        s.value1b(temporary);
        s.value1b(load);
        s.value8b(heartbeat);
    }

    std::string name;
    uint32_t version{ 0 };
    ServiceType type = ServiceTypeUnknown;
    std::string location;
    std::string host;
    uint16_t port = 0;
    std::string ip;
    ServiceStatus status = ServiceStatusOffline;
    timestamp_t startTime = 0;
    timestamp_t stopTime = 0;
    /// Runtime in seconds
    int64_t runTime = 0;
    /// Machine the server is running on
    std::string machine;
    std::string file;
    std::string path;
    std::string arguments;

    /// Temporary service, not written to DB
    bool temporary = false;
    /// Service load, something between 0..100. Not written to DB. The service
    /// is responsible to update this value.
    uint8_t load = 0;
    /// Last heart beat time
    timestamp_t heartbeat{ 0 };
};

}
}
