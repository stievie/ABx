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

#include "stdafx.h"
#include "IOService.h"
#include "Application.h"
#include <AB/Entities/ServiceList.h>
#include <abscommon/DataClient.h>
#include <abscommon/Logger.h>
#include <abscommon/Subsystems.h>
#include <abscommon/UuidUtils.h>
#include <abscommon/MessageClient.h>
#include <sa/ConditionSleep.h>

namespace IO {

bool IOService::GetService(AB::Entities::ServiceType type,
    AB::Entities::Service& service,
    const std::string& preferredUuid /* = Utils::Uuid::EMPTY_UUID */)
{
    DataClient* dc = GetSubsystem<IO::DataClient>();

    AB::Entities::ServiceList sl;
    // Don't cache service list
    dc->Invalidate(sl);
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

bool IOService::EnsureService(AB::Entities::ServiceType type,
    AB::Entities::Service& service,
    const std::string& preferredUuid)
{
    if (GetService(type, service, preferredUuid))
        return true;
    if (!SpawnService(type))
        return false;
    return GetService(type, service, preferredUuid);
}

int IOService::GetServices(AB::Entities::ServiceType type, std::vector<AB::Entities::Service>& services)
{
    DataClient* dc = GetSubsystem<IO::DataClient>();

    AB::Entities::ServiceList sl;
    // Don't cache service list
    dc->Invalidate(sl);
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

bool IOService::SpawnService(AB::Entities::ServiceType type)
{
    auto* msgClient = GetSubsystem<Net::MessageClient>();
    if (!msgClient)
        return false;

    // To be able to spawn a service, there must be one already running of the same type.
    // We tell this service then to spawn anoter instance.
    std::vector<AB::Entities::Service> services;
    if (GetServices(type, services) == 0)
        return false;

    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::Spawn;
    msg.SetBodyString(services[0].uuid);
    if (!msgClient->Write(msg))
        return false;

    bool success = false;
    // Wait for it
    sa::ConditionSleep([oldCount = services.size(), &type, &success]()
    {
        std::vector<AB::Entities::Service> services2;
        if (GetServices(type, services2) > oldCount)
        {
            success = true;
            return true;
        }
        return false;
    }, 5000);
    return success;
}

}
