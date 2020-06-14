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
#include "Player.h"
#include <sa/Assert.h>
#include <sa/Iterator.h>
#include <sa/PropStream.h>

BotClient::BotClient(std::shared_ptr<asio::io_service> ioService, const std::string& loginHost, uint16_t loginPort) :
    client_(*this, ioService)
{
    client_.loginHost_ = loginHost;
    client_.loginPort_ = loginPort;
}

BotClient::~BotClient()
{
}

void BotClient::Update(uint32_t timeElapsed)
{
    client_.Update(timeElapsed, true);
    if (game_)
        game_->Update(timeElapsed);
}

void BotClient::Login()
{
    LOG_INFO << "Logging in with " << username_ << std::endl;
    state_ = State::LoggingIn;
    client_.Login(username_, password_);
}

void BotClient::Logout()
{
    client_.Logout();
    client_.ResetPoll();
    client_.Run();
}

void BotClient::OnLog(const std::string& message)
{
    LOG_INFO << "(" << username_ << "): " << message << std::endl;
}

void BotClient::OnNetworkError(Client::ConnectionError connectionError, const std::error_code& err)
{
    const char* msg = Client::Client::GetNetworkErrorMessage(connectionError);
    LOG_ERROR << "Network error [" << msg << "] (" << username_ << "): (" << err.default_error_condition().value()
              << ") " << err.message() << std::endl;
    state_ = State::Disconnected;
}

void BotClient::OnProtocolError(AB::ErrorCodes err)
{
    LOG_ERROR << "Protocol error (" << username_ << "): " << Client::Client::GetProtocolErrorMessage(err) << std::endl;
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
    LOG_INFO << "Logged in with account UUID " << accountUuid_ << std::endl;
}

void BotClient::OnGetCharlist(const AB::Entities::CharList& chars)
{
    chars_ = chars;
    if (chars_.empty())
    {
        LOG_ERROR << "Account " << username_ << " does not have characters" << std::endl;
        return;
    }

    std::string charUuid;
    std::string mapUuid;

    if (characterName_ != "random")
    {
        const auto it = std::find_if(chars_.begin(), chars_.end(), [this](const AB::Entities::Character& current) {
            return current.name.compare(characterName_) == 0;
        });
        if (it == chars_.end())
        {
            LOG_ERROR << "Account " << username_ << " does not have a character with name " << characterName_
                      << std::endl;
            return;
        }
        charUuid = (*it).uuid;
        mapUuid = (*it).lastOutpostUuid;
        currentName_ = characterName_;
    }
    else
    {
        const auto it = sa::SelectRandomly(chars_.begin(), chars_.end());
        assert(it != chars_.end());
        charUuid = (*it).uuid;
        mapUuid = (*it).lastOutpostUuid;
        currentName_ = (*it).name;
    }

    client_.EnterWorld(charUuid, mapUuid);
}

void BotClient::OnGetOutposts(const std::vector<AB::Entities::Game>& outposts)
{
    outposts_ = outposts;
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
    LOG_DEBUG << "ChangeInstance" << std::endl;
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::EnterWorld& packet)
{
    state_ = State::World;
    const auto it = std::find_if(outposts_.begin(), outposts_.end(), [&packet](const AB::Entities::Game& current) {
        return current.uuid.compare(packet.mapUuid) == 0;
    });
    const std::string mapName = (it != outposts_.end() ? (*it).name : "Unknown");
    LOG_INFO << currentName_ << " entered " << mapName << std::endl;
    playerId_ = packet.playerId;
    game_ = std::make_unique<Game>(static_cast<AB::Entities::GameType>(packet.gameType));
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::PlayerAutorun&)
{
}

void BotClient::SpawnObject(AB::GameProtocol::GameObjectType type, uint32_t id,
    const Math::Vector3& pos, const Math::Vector3& scale, const Math::Quaternion& rot,
    const std::string data)
{
    std::unique_ptr<GameObject> object;
    switch (type)
    {
    case AB::GameProtocol::GameObjectType::Unknown:
    case AB::GameProtocol::GameObjectType::Static:
    case AB::GameProtocol::GameObjectType::TerrainPatch:
    case AB::GameProtocol::GameObjectType::__SentToPlayer:
        break;
    case AB::GameProtocol::GameObjectType::ItemDrop:
        object = std::make_unique<GameObject>(GameObject::Type::ItemDrop, id);
        break;
    case AB::GameProtocol::GameObjectType::AreaOfEffect:
        object = std::make_unique<GameObject>(GameObject::Type::AOE, id);
        break;
    case AB::GameProtocol::GameObjectType::Projectile:
        object = std::make_unique<GameObject>(GameObject::Type::Projectile, id);
        break;
    case AB::GameProtocol::GameObjectType::Npc:
        object = std::make_unique<GameObject>(GameObject::Type::Npc, id);
        break;
    case AB::GameProtocol::GameObjectType::Player:
        if (id == playerId_)
            object = std::make_unique<Player>(GameObject::Type::Self, id, client_, script_);
        else
            object = std::make_unique<GameObject>(GameObject::Type::Player, id);
        break;
    }
    if (object)
    {
        sa::PropReadStream stream(data.c_str(), data.length());
        object->transformation_.position_ = pos;
        object->transformation_.scale_ = scale;
        object->transformation_.oriention_ = rot;
        object->SetData(stream);
        game_->AddObject(std::move(object));
    }
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSpawn& packet)
{
    Math::Quaternion direction = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, packet.rot);
    SpawnObject(static_cast<AB::GameProtocol::GameObjectType>(packet.type), packet.id,
        packet.pos, packet.scale, direction, packet.data);
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSpawnExisting& packet)
{
    Math::Quaternion direction = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, packet.rot);
    SpawnObject(static_cast<AB::GameProtocol::GameObjectType>(packet.type), packet.id,
        packet.pos, packet.scale, direction, packet.data);
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::MailHeaders&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::MailComplete&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectDespawn& packet)
{
    game_->RemoveObject(packet.id);
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectPositionUpdate& packet)
{
    auto* object = game_->GetObject(packet.id);
    if (!object)
        return;
    object->transformation_.position_ = packet.pos;
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

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectRotationUpdate& packet)
{
    auto* object = game_->GetObject(packet.id);
    if (object)
    {
        object->transformation_.SetYRotation(packet.yRot);
    }
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectTargetSelected&)
{
}

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectStateChanged& packet)
{
    auto* object = game_->GetObject(packet.id);
    if (object)
        object->OnStateChanged(packet.state);
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

void BotClient::OnPacket(int64_t, const AB::Packets::Server::ObjectForcePosition& packet)
{
    auto* object = game_->GetObject(packet.id);
    if (!object)
        return;
    object->transformation_.position_ = packet.pos;
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
