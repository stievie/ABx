#pragma once

#include <AB/Entities/Service.h>
#include "UuidUtils.h"

namespace IO {

class IOService
{
public:
    IOService() = delete;
    ~IOService() = delete;

    /// Get the service with the least load.
    static bool GetService(AB::Entities::ServiceType type,
        AB::Entities::Service& service,
        const std::string& preferredUuid = Utils::Uuid::EMPTY_UUID);
    static int GetServices(AB::Entities::ServiceType type,
        std::vector<AB::Entities::Service>& services);
};

}
