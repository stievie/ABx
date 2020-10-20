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

#include <system_error>
#include <stdint.h>
#include <string>
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include <AB/Entities/GuildMembers.h>
#include <AB/Entities/Guild.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include "Errors.h"
#include <set>
#include <sa/Compiler.h>

namespace Client {

inline bool IsOffline(AB::Packets::Server::PlayerInfo::Status status)
{
    return (status == AB::Packets::Server::PlayerInfo::OnlineStatusOffline) ||
        (status == AB::Packets::Server::PlayerInfo::OnlineStatusInvisible);
}

class SA_NOVTABLE Receiver
{
public:
    virtual ~Receiver() = default;
    virtual void OnLog(const std::string& message) = 0;
    virtual void OnNetworkError(ConnectionError connectionError, const std::error_code& err) = 0;
    virtual void OnProtocolError(AB::ErrorCodes err) = 0;
    virtual void OnHttpError(int status) { (void)status; };
    virtual void OnPong(int lastPing) = 0;

    virtual void OnLoggedIn(const std::string& accountUuid, const std::string& authToken, AB::Entities::AccountType accType) = 0;
    virtual void OnGetCharlist(const AB::Entities::CharList& chars) = 0;
    virtual void OnGetOutposts(const std::vector<AB::Entities::Game>& games) = 0;
    virtual void OnGetServices(const std::vector<AB::Entities::Service>& services) = 0;
    virtual void OnAccountCreated() = 0;
    virtual void OnPlayerCreated(const std::string& uuid, const std::string& mapUuid) = 0;
    virtual void OnAccountKeyAdded() = 0;
    virtual void OnCharacterDeleted(const std::string& uuid) = 0;

    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerJoined& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerLeft& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChangeInstance& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::EnterWorld& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerAutorun& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawn& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawnExisting& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::MailHeaders& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::MailComplete& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDespawn& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPositionUpdate& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpeedChanged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryContent& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemUpdate& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemDelete& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestContent& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemUpdate& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemDelete& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectRotationUpdate& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectTargetSelected& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectStateChanged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerError& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillFailure& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectUseSkill& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillSuccess& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectAttackFailure& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPingTarget& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectAdded& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectRemoved& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDamaged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectHealed& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectProgress& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDroppedItem& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectForcePosition& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectGroupMaskChanged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetAttackSpeed& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerMessage& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChatMessage& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerInvited& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerRemoved& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerAdded& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyInviteRemoved& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyResigned& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyDefeated& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyMembersInfo& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectResourceChanged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::DialogTrigger& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::FriendList& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerInfo& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::FriendAdded& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::FriendRemoved& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::FriendRenamed& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::GuildInfo& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::GuildMemberList& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestSelectionDialogTrigger& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDialogTrigger& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::NpcHasQuest& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDeleted& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestRewarded& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::SetObjectAttributeValue& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSecProfessionChanged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetSkill& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::SkillTemplateLoaded& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::TradeDialogTrigger& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::TradeCancel& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::TradeOffer& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::TradeAccepted& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::MerchantItems& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ItemPrice& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::CraftsmanItems& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::DropTargetChanged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PositionPinged& packet) = 0;

};

}
