#include "stdafx.h"
#include "ProtocolStatus.h"
#include "Utils.h"
#include "ConfigManager.h"
#include "Dispatcher.h"
#include "OutputMessage.h"
#include <pugixml.hpp>
#include <ostream>
#include <iostream>

#include "DebugNew.h"

namespace Net {

std::map<uint32_t, int64_t> ProtocolStatus::ipConnectMap_;

enum RequestInfo_t : uint16_t
{
    BasicServerInfo    = 1 << 0,
    OwnerServerInfo    = 1 << 1,
    MiscServerInfo     = 1 << 2,
    PlayersInfo        = 1 << 3,
    MapInfo            = 1 << 4,
    ExtPlayersInfo     = 1 << 5,
    PlayerStatusInfo   = 1 << 6,
    ServerSoftwareInfo = 1 << 7
};

void ProtocolStatus::OnRecvFirstMessage(NetworkMessage& message)
{
    MapIter it = ipConnectMap_.find(GetIP());
    if (it != ipConnectMap_.end())
    {
        if (Utils::AbTick() < it->second + ConfigManager::Instance[ConfigManager::Key::StatusQueryTimeout].GetInt())
        {
            Disconnect();
        }
    }
    ipConnectMap_[GetIP()] = Utils::AbTick();

    switch (message.GetByte())
    {
    case 0xFF:
        // XML Info
        if (message.GetString(4) == "info")
        {
            Asynch::Dispatcher::Instance.Add(
                Asynch::CreateTask(std::bind(&ProtocolStatus::SendStatusString, this))
            );
            return;
        }
        break;
    case 0x01:
    {
        uint16_t requestInfo = message.Get<uint16_t>();
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(std::bind(&ProtocolStatus::SendInfo, this, requestInfo))
        );
        return;
    }
    default:
        break;
    }

    Disconnect();
}

void ProtocolStatus::SendStatusString()
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    pugi::xml_document doc;
    pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version") = "1.0";

    pugi::xml_node tsqp = doc.append_child("ab");
    tsqp.append_attribute("version") = "1.0";

    std::ostringstream ss;
    doc.save(ss, "", pugi::format_raw);

    std::string data = ss.str();
    output->AddBytes(data.c_str(), static_cast<uint32_t>(data.size()));

    Send(output);
    Disconnect();
}

void ProtocolStatus::SendInfo(uint16_t requestedInfo)
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (requestedInfo & BasicServerInfo)
    {
        output->AddByte(0x10);
        output->AddString(ConfigManager::Instance[ConfigManager::Key::ServerName].ToString());
        output->AddString(ConfigManager::Instance[ConfigManager::Key::IP].ToString());
        output->AddString(ConfigManager::Instance[ConfigManager::Key::LoginPort].ToString());
    }

    Send(output);
    Disconnect();
}

}
