#include "stdafx.h"
#include "IOService.h"
#include <AB/Entities/ServiceList.h>
#include "Application.h"
#include "Logger.h"

namespace IO {

std::pair<std::string, uint16_t> IOService::GetService(AB::Entities::ServiceType type,
    const std::string& preferredUuid /* = "00000000-0000-0000-0000-000000000000" */)
{
    DataClient* dc = Application::Instance->GetDataClient();

    AB::Entities::ServiceList sl;
    if (!dc->Read(sl))
        return std::pair<std::string, uint16_t>();

    std::vector<AB::Entities::Service> services;

    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dc->Read(s))
            continue;
        if (s.status != AB::Entities::ServiceStatusOnline)
            continue;
        if (s.type == type)
        {
            // Use preferred server is possible
            if (s.uuid.compare(preferredUuid) == 0 && s.load < 90)
                return{ s.host, s.port };
            services.push_back(s);
        }
    }

    if (services.size() != 0)
    {
        std::sort(services.begin(), services.end(),
            [](AB::Entities::Service const& a, AB::Entities::Service const& b)
        {
            return a.load < b.load;
        });
        return{ services[0].host, services[0].port };
    }

    LOG_WARNING << "No server of type " << static_cast<int>(type) << " online" << std::endl;
    return std::pair<std::string, uint16_t>();
}

}
