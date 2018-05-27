#pragma once

#include <AB/Entities/Service.h>

namespace IO {

class IOService
{
public:
    IOService() = delete;
    ~IOService() = delete;

    static std::pair<std::string, uint16_t> GetService(AB::Entities::ServiceType type);
};

}
