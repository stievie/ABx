#include "stdafx.h"
#include "IOService.h"
#include <AB/Entities/ServiceList.h>
#include "Application.h"
#include "Logger.h"
#include "Subsystems.h"
#include "DataClient.h"
#include "UuidUtils.h"

namespace IO {

bool IOService::GetService(AB::Entities::ServiceType type,
    AB::Entities::Service& service,
    const std::string& preferredUuid /* = Utils::Uuid::EMPTY_UUID */)
{
    DataClient* dc = GetSubsystem<IO::DataClient>();

    AB::Entities::ServiceList sl;
    if (!dc->Read(sl))
    {
        LOG_ERROR << "Error reading service list" << std::endl;
        return false;
    }

    std::vector<AB::Entities::Service> services;

    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dc->Read(s))
            continue;
        if (s.status != AB::Entities::ServiceStatusOnline)
            continue;
        if (s.type == AB::Entities::ServiceTypeFileServer || s.type == AB::Entities::ServiceTypeGameServer)
        {
            // File and game server send a heart beat. Look if they are still alive.
            if (Utils::TimeElapsed(s.heartbeat) > AB::Entities::HEARTBEAT_INTERVAL * 2)
            {
                // Maybe dead
                LOG_INFO << "No heart beat from service " << s.uuid << " for " << Utils::TimeElapsed(s.heartbeat) << " ms" << std::endl;
                continue;
            }
        }
        if (s.type == type)
        {
            // Use preferred server is possible
            if (Utils::Uuid::IsEqual(s.uuid, preferredUuid) == 0 && s.load < 90)
            {
                service = s;
                return true;
            }
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
        if (services[0].type == AB::Entities::ServiceTypeFileServer || services[0].load < 100)
        {
            service = services[0];
            return true;
        }
    }

    LOG_WARNING << "No server of type " << static_cast<int>(type) << " online" << std::endl;
    return false;
}

int IOService::GetServices(AB::Entities::ServiceType type, std::vector<AB::Entities::Service>& services)
{
    DataClient* dc = GetSubsystem<IO::DataClient>();

    AB::Entities::ServiceList sl;
    if (!dc->Read(sl))
    {
        LOG_ERROR << "Error reading service list" << std::endl;
        return 0;
    }

    int result = 0;
    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dc->Read(s))
        {
            LOG_ERROR << "Error service with UUID " << uuid << std::endl;
            continue;
        }
        if (s.status != AB::Entities::ServiceStatusOnline)
            continue;
        if (s.type == type)
        {
            services.push_back(s);
            ++result;
        }
    }
    return result;
}

}
