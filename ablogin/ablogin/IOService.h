#pragma once

#include <AB/Entities/Service.h>

namespace IO {

class IOService
{
public:
    IOService() = delete;
    ~IOService() = delete;

    /// Get the service with the least load.
    static bool GetService(AB::Entities::ServiceType type,
        AB::Entities::Service& service,
        const std::string& preferredUuid = "00000000-0000-0000-0000-000000000000");
};

}
