#include "stdafx.h"
#include "IOService.h"
#include <AB/Entities/ServiceList.h>
#include "Application.h"

namespace IO {

std::pair<std::string, uint16_t> IOService::GetService(AB::Entities::ServiceType type)
{
    DataClient* dc = Application::Instance->GetDataClient();

    AB::Entities::ServiceList sl;
    if (!dc->Read(sl))
        return std::pair<std::string, uint16_t>();

    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dc->Read(s))
            continue;
        if (s.type == type)
            return std::make_pair(s.host, s.port);
    }
    return std::pair<std::string, uint16_t>();
}

}
