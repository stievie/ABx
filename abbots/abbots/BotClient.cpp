/**
 * Copyright 2020 Stefan Ascher
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

#include "BotClient.h"
#include <abscommon/Logger.h>
#include "Game.h"
#include "GameObject.h"

BotClient::BotClient(std::shared_ptr<asio::io_service> ioService) :
    client_(*this, ioService)
{
}

BotClient::~BotClient()
{
}

void BotClient::Update(uint32_t timeElapsed)
{
    if (game_)
        game_->Update(timeElapsed);
}

void BotClient::Login()
{
    LOG_INFO << "Logging in with " << username_ << std::endl;
    client_.Login(username_, password_);
}

void BotClient::Logout()
{
    client_.Logout();
}

void BotClient::OnLog(const std::string& message)
{
    LOG_INFO << message << std::endl;
}

void BotClient::OnNetworkError(Client::ConnectionError, const std::error_code&)
{
}

void BotClient::OnProtocolError(AB::ErrorCodes)
{
}

void BotClient::OnPong(int lastPing)
{
    (void)lastPing;
}

void BotClient::OnLoggedIn(const std::string& accountUuid,
    const std::string& authToken,
    AB::Entities::AccountType accType)
{
    accountUuid_ = accountUuid;
    authToken_ = authToken;
    accountType_ = accType;
}

void BotClient::OnGetCharlist(const AB::Entities::CharList& chars)
{
    chars_ = chars;
}

void BotClient::OnGetOutposts(const std::vector<AB::Entities::Game>&)
{
}

void BotClient::OnGetServices(const std::vector<AB::Entities::Service>&)
{
}

void BotClient::OnAccountCreated()
{
}

void BotClient::OnPlayerCreated(const std::string&, const std::string&)
{
}

void BotClient::OnAccountKeyAdded()
{
}

void BotClient::OnCharacterDeleted(const std::string&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ServerJoined&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ServerLeft&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ChangeInstance&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::EnterWorld&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PlayerAutorun&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSpawn&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSpawnExisting&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::MailHeaders&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::MailComplete&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectDespawn&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectPositionUpdate&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSpeedChanged&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::InventoryContent&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::InventoryItemUpdate&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::InventoryItemDelete&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ChestContent&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ChestItemUpdate&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ChestItemDelete&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectRotationUpdate&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectTargetSelected&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectStateChanged&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PlayerError&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSkillFailure&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectUseSkill&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSkillSuccess&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectAttackFailure&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectPingTarget&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectEffectAdded&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectEffectRemoved&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectDamaged&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectHealed&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectProgress&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectDroppedItem&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectForcePosition&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectGroupMaskChanged&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSetAttackSpeed&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ServerMessage&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ChatMessage&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PartyPlayerInvited&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PartyPlayerRemoved&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PartyPlayerAdded&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PartyInviteRemoved&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PartyResigned&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PartyDefeated&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PartyMembersInfo&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectResourceChanged&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::DialogTrigger&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::FriendList&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PlayerInfo&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::FriendAdded&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::FriendRemoved&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::FriendRenamed&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::GuildInfo&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::GuildMemberList&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::QuestSelectionDialogTrigger&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::QuestDialogTrigger&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::NpcHasQuest&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::QuestDeleted&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::QuestRewarded&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::SetObjectAttributeValue&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSecProfessionChanged&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSetSkill&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::SkillTemplateLoaded&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::TradeDialogTrigger&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::TradeCancel&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::TradeOffer&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::TradeAccepted&)
{
}
