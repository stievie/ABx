#include "stdafx.h"
#include "ProtocolStatus.h"
#include "Utils.h"
#include "ConfigManager.h"

#include "DebugNew.h"

namespace Net {

std::map<uint32_t, int64_t> ProtocolStatus::ipConnectMap_;

ProtocolStatus::ProtocolStatus(std::shared_ptr<Connection> connection) :
    Protocol(connection)
{
}

ProtocolStatus::~ProtocolStatus()
{
}

void ProtocolStatus::OnRecvFirstMessage(NetworkMessage& message)
{
    MapIter it = ipConnectMap_.find(GetIP());
    if (it != ipConnectMap_.end())
    {
        if (Utils::AbTick() < it->second + ConfigManager::Instance.config_[ConfigManager::Key::StatusQueryTimeout].GetInt())
        {
            GetConnection()->Close();
        }
    }
    ipConnectMap_[GetIP()] = Utils::AbTick();

    switch (message.GetByte())
    {
    case 0xFF:
        break;
    default:
        break;
    }

    GetConnection()->Close();
}

}
