#include "stdafx.h"
#include "Client.h"
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include "Connection.h"
#define USE_STANDALONE_ASIO
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4457 4456 4150)
#endif
#include <SimpleWeb/client_https.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <iostream>
#include <fstream>

namespace Client {

class HttpsClient : public SimpleWeb::Client<SimpleWeb::HTTPS>
{
public:
    HttpsClient(const std::string &server_port_path, bool verify_certificate = true,
        const std::string &cert_file = std::string(),
        const std::string &private_key_file = std::string(),
        const std::string &verify_file = std::string()) :
        SimpleWeb::Client<SimpleWeb::HTTPS>::Client(server_port_path, verify_certificate,
            cert_file, private_key_file, verify_file)
    { }
};

Client::Client(Receiver& receiver) :
    receiver_(receiver),
    ioService_(std::make_shared<asio::io_service>()),
    protoLogin_(nullptr),
    protoGame_(nullptr),
    lastRun_(0),
    lastPing_(0),
    gotPong_(true),
    loginHost_("127.0.0.1"),
    loginPort_(2748),
    fileHost_(""),
    filePort_(0),
    gameHost_(""),
    gamePort_(0),
    state_(ClientState::Disconnected),
    httpClient_(nullptr)
{
    // Always create new keys
    arc4random_stir();
    dhKeys_.GenerateKeys();
}

Client::~Client()
{
    if (httpClient_)
        delete httpClient_;
    Terminate();
}

void Client::ResetPoll()
{
    // Blocking!
    // Reset must always be called prior to poll
    ioService_->reset();
    ioService_->poll();
}

void Client::Poll()
{
    ioService_->poll();
}

void Client::Run()
{
#ifndef _WIN32
    // WTF, why is this needed on Linux but not on Windows?
    if (ioService_->stopped())
        ioService_->reset();
#endif // _WIN32
    ioService_->run();
}

void Client::Terminate()
{
    ioService_->stop();
    Connection::Terminate();
}

void Client::OnLoggedIn(const std::string& accountUuid, const std::string& authToken)
{
    accountUuid_ = accountUuid;
    authToken_ = authToken;
    gamePort_ = protoLogin_->gamePort_;
    if (!protoLogin_->gameHost_.empty())
        gameHost_ = protoLogin_->gameHost_;
    else
        // If game host is empty use the login host
        gameHost_ = loginHost_;

    filePort_ = protoLogin_->filePort_;
    if (!protoLogin_->fileHost_.empty())
        fileHost_ = protoLogin_->fileHost_;
    else
        // If file host is empty use the login host
        fileHost_ = loginHost_;

    if (!fileHost_.empty() && filePort_ != 0)
    {
        std::stringstream ss;
        ss << fileHost_ << ":" << filePort_;
        httpClient_ = new HttpsClient(ss.str(), false);
    }

    receiver_.OnLoggedIn(accountUuid_, authToken_);
}

void Client::OnGetCharlist(const AB::Entities::CharList& chars)
{
    state_ = ClientState::SelectChar;

    receiver_.OnGetCharlist(chars);

    // Get list of outposts
    GetOutposts();
}

void Client::OnGetOutposts(const std::vector<AB::Entities::Game>& games)
{

    receiver_.OnGetOutposts(games);
}

void Client::OnGetServices(const std::vector<AB::Entities::Service>& services)
{
    receiver_.OnGetServices(services);
}

void Client::OnGetMailHeaders(int64_t updateTick, const std::vector<AB::Entities::MailHeader>& headers)
{
    receiver_.OnGetMailHeaders(updateTick, headers);
}

void Client::OnGetMail(int64_t updateTick, const AB::Entities::Mail& mail)
{
    receiver_.OnGetMail(updateTick, mail);
}

void Client::OnGetInventory(int64_t updateTick, const std::vector<InventoryItem>& items)
{
    receiver_.OnGetInventory(updateTick, items);
}

void Client::OnInventoryItemUpdate(int64_t updateTick, const InventoryItem& item)
{
    receiver_.OnInventoryItemUpdate(updateTick, item);
}

void Client::OnInventoryItemDelete(int64_t updateTick, uint16_t pos)
{
    receiver_.OnInventoryItemDelete(updateTick, pos);
}

void Client::OnGetChest(int64_t updateTick, const std::vector<InventoryItem>& items)
{
    receiver_.OnGetChest(updateTick, items);
}

void Client::OnChestItemUpdate(int64_t updateTick, const InventoryItem& item)
{
    receiver_.OnChestItemUpdate(updateTick, item);
}

void Client::OnChestItemDelete(int64_t updateTick, uint16_t pos)
{
    receiver_.OnChestItemDelete(updateTick, pos);
}

void Client::OnEnterWorld(int64_t updateTick, const std::string& serverId,
    const std::string& mapUuid, const std::string& instanceUuid, uint32_t playerId,
    AB::Entities::GameType type, uint8_t partySize)
{
    state_ = ClientState::World;
    mapUuid_ = mapUuid;

    receiver_.OnEnterWorld(updateTick, serverId, mapUuid, instanceUuid, playerId, type, partySize);
}

void Client::OnChangeInstance(int64_t updateTick, const std::string& serverId,
    const std::string& mapUuid, const std::string& instanceUuid, const std::string& charUuid)
{
    receiver_.OnChangeInstance(updateTick, serverId, mapUuid, instanceUuid, charUuid);
}

void Client::OnDespawnObject(int64_t updateTick, uint32_t id)
{
    receiver_.OnDespawnObject(updateTick, id);
}

void Client::OnObjectPos(int64_t updateTick, uint32_t id, const Vec3& pos)
{
    receiver_.OnObjectPos(updateTick, id, pos);
}

void Client::OnObjectRot(int64_t updateTick, uint32_t id, float rot, bool manual)
{
    receiver_.OnObjectRot(updateTick, id, rot, manual);
}

void Client::OnObjectStateChange(int64_t updateTick, uint32_t id, AB::GameProtocol::CreatureState state)
{
    receiver_.OnObjectStateChange(updateTick, id, state);
}

void Client::OnObjectSpeedChange(int64_t updateTick, uint32_t id, float speedFactor)
{
    receiver_.OnObjectSpeedChange(updateTick, id, speedFactor);
}

void Client::OnAccountCreated()
{
    receiver_.OnAccountCreated();
}

void Client::OnPlayerCreated(const std::string& uuid, const std::string& mapUuid)
{
    receiver_.OnPlayerCreated(uuid, mapUuid);
}

void Client::OnObjectSelected(int64_t updateTick, uint32_t sourceId, uint32_t targetId)
{
    receiver_.OnObjectSelected(updateTick, sourceId, targetId);
}

void Client::OnObjectSkillFailure(int64_t updateTick, uint32_t id, int skillIndex, AB::GameProtocol::SkillError error)
{
    receiver_.OnObjectSkillFailure(updateTick, id, skillIndex, error);
}

void Client::OnObjectAttackFailure(int64_t updateTick, uint32_t id, AB::GameProtocol::AttackError error)
{
    receiver_.OnObjectAttackFailure(updateTick, id, error);
}

void Client::OnObjectUseSkill(int64_t updateTick, uint32_t id, int skillIndex,
    uint16_t energy, uint16_t adrenaline, uint16_t activation, uint16_t overcast, uint16_t hp)
{
    receiver_.OnObjectUseSkill(updateTick, id, skillIndex, energy, adrenaline, activation, overcast, hp);
}

void Client::OnObjectEndUseSkill(int64_t updateTick, uint32_t id, int skillIndex, uint16_t recharge)
{
    receiver_.OnObjectEndUseSkill(updateTick, id, skillIndex, recharge);
}

void Client::OnObjectPingTarget(int64_t updateTick, uint32_t id, uint32_t targetId, AB::GameProtocol::ObjectCallType type, int skillIndex)
{
    receiver_.OnObjectPingTarget(updateTick, id, targetId, type, skillIndex);
}

void Client::OnObjectEffectAdded(int64_t updateTick, uint32_t id, uint32_t effectIndex, uint32_t ticks)
{
    receiver_.OnObjectEffectAdded(updateTick, id, effectIndex, ticks);
}

void Client::OnObjectEffectRemoved(int64_t updateTick, uint32_t id, uint32_t effectIndex)
{
    receiver_.OnObjectEffectRemoved(updateTick, id, effectIndex);
}

void Client::OnObjectDamaged(int64_t updateTick, uint32_t id, uint32_t sourceId, uint16_t index, uint8_t damageType, int16_t value)
{
    receiver_.OnObjectDamaged(updateTick, id, sourceId, index, damageType, value);
}

void Client::OnObjectHealed(int64_t updateTick, uint32_t id, uint32_t healerId, uint16_t index, int16_t value)
{
    receiver_.OnObjectHealed(updateTick, id, healerId, index, value);
}

void Client::OnObjectProgress(int64_t updateTick, uint32_t id, AB::GameProtocol::ObjectProgressType type, int value)
{
    receiver_.OnObjectProgress(updateTick, id, type, value);
}

void Client::OnObjectDroppedItem(int64_t updateTick, uint32_t id, uint32_t targetId, uint32_t itemId,
    uint32_t itemIndex, uint32_t count, uint16_t value)
{
    receiver_.OnObjectDroppedItem(updateTick, id, targetId, itemId, itemIndex, count, value);
}

void Client::OnResourceChanged(int64_t updateTick, uint32_t id,
    AB::GameProtocol::ResourceType resType, int16_t value)
{
    receiver_.OnResourceChanged(updateTick, id, resType, value);
}

void Client::OnObjectSetPosition(int64_t updateTick, uint32_t id, const Vec3& pos)
{
    receiver_.OnObjectSetPosition(updateTick, id, pos);
}

void Client::OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
    const std::string& senderName, const std::string& message)
{
    receiver_.OnServerMessage(updateTick, type, senderName, message);
}

void Client::OnChatMessage(int64_t updateTick, AB::GameProtocol::ChatMessageChannel channel,
    uint32_t senderId, const std::string& senderName, const std::string& message)
{
    receiver_.OnChatMessage(updateTick, channel, senderId, senderName, message);
}

void Client::OnPlayerError(int64_t updateTick, AB::GameProtocol::PlayerErrorValue error)
{
    receiver_.OnPlayerError(updateTick, error);
}

void Client::OnPlayerAutorun(int64_t updateTick, bool autorun)
{
    receiver_.OnPlayerAutorun(updateTick, autorun);
}

void Client::OnPartyInvited(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId)
{
    receiver_.OnPartyInvited(updateTick, sourceId, targetId, partyId);
}

void Client::OnPartyRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId)
{
    receiver_.OnPartyRemoved(updateTick, sourceId, targetId, partyId);
}

void Client::OnPartyAdded(int64_t updateTick, uint32_t acceptorId, uint32_t leaderId, uint32_t partyId)
{
    receiver_.OnPartyAdded(updateTick, acceptorId, leaderId, partyId);
}

void Client::OnPartyInviteRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId)
{
    receiver_.OnPartyInviteRemoved(updateTick, sourceId, targetId, partyId);
}

void Client::OnPartyResigned(int64_t updateTick, uint32_t partyId)
{
    receiver_.OnPartyResigned(updateTick, partyId);
}

void Client::OnPartyDefeated(int64_t updateTick, uint32_t partyId)
{
    receiver_.OnPartyDefeated(updateTick, partyId);
}

void Client::OnPartyInfoMembers(uint32_t partyId, const std::vector<uint32_t>& members)
{
    receiver_.OnPartyInfoMembers(partyId, members);
}

void Client::OnDialogTrigger(int64_t updateTick, uint32_t dialogId)
{
    receiver_.OnDialogTrigger(updateTick, dialogId);
}

void Client::OnPlayerLoggedIn(int64_t updateTick, const RelatedAccount& player)
{
    receiver_.OnPlayerLoggedIn(updateTick, player);
}

void Client::OnPlayerLoggedOut(int64_t updateTick, const RelatedAccount& player)
{
    receiver_.OnPlayerLoggedOut(updateTick, player);
}

void Client::OnPlayerInfo(int64_t updateTick, const RelatedAccount& player)
{
    receiver_.OnPlayerInfo(updateTick, player);
}

void Client::OnFriendList(int64_t updateTick, const std::vector<std::string>& list)
{
    receiver_.OnFriendList(updateTick, list);
}

void Client::OnFriendAdded(int64_t updateTick, const std::string& accountUuid, RelatedAccount::Releation relation)
{
    receiver_.OnFriendAdded(updateTick, accountUuid, relation);
}

void Client::OnFriendRemoved(int64_t updateTick, const std::string& accountUuid, RelatedAccount::Releation relation)
{
    receiver_.OnFriendRemoved(updateTick, accountUuid, relation);
}

void Client::OnGuildMemberList(int64_t updateTick, const std::vector<std::string>& list)
{
    receiver_.OnGuildMemberList(updateTick, list);
}

void Client::OnGuildInfo(int64_t updateTick, const AB::Entities::Guild& guild)
{
    receiver_.OnGuildInfo(updateTick, guild);
}

void Client::OnSpawnObject(int64_t updateTick, uint32_t id, const ObjectSpawn& objectSpawn,
    PropReadStream& data, bool existing)
{
    receiver_.OnSpawnObject(updateTick, id, objectSpawn, data, existing);
}

void Client::OnLog(const std::string& message)
{
    receiver_.OnLog(message);
}

void Client::OnNetworkError(ConnectionError connectionError, const std::error_code& err)
{
    receiver_.OnNetworkError(connectionError, err);
}

void Client::OnProtocolError(uint8_t err)
{
    receiver_.OnProtocolError(err);
}

void Client::OnPong(int lastPing)
{
    gotPong_ = true;
    while (pings_.size() > 9)
        pings_.erase(pings_.begin());
    pings_.push_back(lastPing);
}

void Client::OnServerJoined(const AB::Entities::Service& service)
{
    receiver_.OnServerJoined(service);
}

void Client::OnServerLeft(const AB::Entities::Service& service)
{
    receiver_.OnServerLeft(service);
}

std::shared_ptr<ProtocolLogin> Client::GetProtoLogin()
{
    if (!protoLogin_)
    {
        protoLogin_ = std::make_shared<ProtocolLogin>(dhKeys_, *ioService_.get());
        protoLogin_->SetErrorCallback(std::bind(&Client::OnNetworkError, this, std::placeholders::_1, std::placeholders::_2));
        protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    }
    return protoLogin_;
}

void Client::Login(const std::string& name, const std::string& pass)
{
    if (!(state_ == ClientState::Disconnected || state_ == ClientState::CreateAccount))
        return;

    accountName_ = name;
    password_ = pass;

    // 1. Login to login server -> get character list
    GetProtoLogin()->Login(loginHost_, loginPort_, name, pass,
        std::bind(&Client::OnLoggedIn, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&Client::OnGetCharlist, this, std::placeholders::_1));
}

void Client::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    if (state_ != ClientState::CreateAccount)
        return;

    accountName_ = name;
    password_ = pass;

    GetProtoLogin()->CreateAccount(loginHost_, loginPort_, name, pass,
        email, accKey,
        std::bind(&Client::OnAccountCreated, this));
}

void Client::CreatePlayer(const std::string& charName, const std::string& profUuid,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex, bool isPvp)
{
    if (state_ != ClientState::SelectChar)
        return;

    if (accountUuid_.empty() || password_.empty())
        return;

    GetProtoLogin()->CreatePlayer(loginHost_, loginPort_, accountUuid_, authToken_,
        charName, profUuid, modelIndex, sex, isPvp,
        std::bind(&Client::OnPlayerCreated, this, std::placeholders::_1, std::placeholders::_2));
}

void Client::Logout()
{
    if (state_ != ClientState::World)
        return;
    if (protoGame_)
    {
        state_ = ClientState::Disconnected;
        protoGame_->Logout();
        Run();
    }
}

void Client::GetOutposts()
{
    if (accountUuid_.empty() || password_.empty())
        return;

    GetProtoLogin()->GetOutposts(loginHost_, loginPort_, accountUuid_, authToken_,
        std::bind(&Client::OnGetOutposts, this, std::placeholders::_1));
}

void Client::GetServers()
{
    if (accountUuid_.empty() || password_.empty())
        return;

    GetProtoLogin()->GetServers(loginHost_, loginPort_, accountUuid_, authToken_,
        std::bind(&Client::OnGetServices, this, std::placeholders::_1));
}

void Client::EnterWorld(const std::string& charUuid, const std::string& mapUuid,
    const std::string& host /* = "" */, uint16_t port /* = 0 */, const std::string& instanceId /* = "" */)
{
    assert(!accountUuid_.empty());
    // Enter or changing the world
    if (state_ != ClientState::SelectChar && state_ != ClientState::World)
        return;

    if (state_ == ClientState::World)
    {
        // We are already logged in to some world so we must logout
        Logout();
        gotPong_ = true;
    }

    // Maybe different server
    if (!host.empty())
        gameHost_ = host;
    if (port != 0)
        gamePort_ = port;

    // 2. Login to game server
    if (!protoGame_)
        protoGame_ = std::make_shared<ProtocolGame>(*this, dhKeys_, *ioService_.get());

    protoGame_->Login(accountUuid_, authToken_, charUuid, mapUuid, instanceId,
        gameHost_, gamePort_);
}

void Client::Update(int timeElapsed)
{
    if (state_ == ClientState::World)
    {
        if ((lastPing_ >= 1000 && gotPong_) || (lastPing_ > 5000))
        {
            // Send every second a Ping. If we didn't get a pong the last 5 seconds also send a ping.
            if (protoGame_)
            {
                gotPong_ = false;
                protoGame_->Ping();
            }
            lastPing_ = 0;
        }
    }

    lastRun_ += timeElapsed;
    if (lastRun_ >= 16)
    {
        // Don't send more than ~60 updates to the server, it might DC.
        // If running @144Hz every 2nd Update. If running @60Hz every update
        lastRun_ = 0;
        Run();
    }
    if (state_ == ClientState::World)
        lastPing_ += timeElapsed;
}

bool Client::HttpRequest(const std::string& path, std::ostream& out)
{
    if (httpClient_ == nullptr)
        return false;
    SimpleWeb::CaseInsensitiveMultimap header;
    header.emplace("Connection", "keep-alive");
    std::stringstream ss;
    ss << accountUuid_ << authToken_;
    header.emplace("Auth", ss.str());
    try
    {
        auto r = httpClient_->request("GET", path, "", header);
        if (r->status_code != "200 OK")
            return false;
        out << r->content.rdbuf();
        return true;
    }
    catch (const SimpleWeb::system_error&)
    {
        return false;
    }
}

bool Client::HttpDownload(const std::string& path, const std::string& outFile)
{
    std::remove(outFile.c_str());
    std::ofstream f;
    f.open(outFile);
    if (!f.is_open())
        return false;
    bool ret = HttpRequest(path, f);
    f.close();
    return ret;
}

uint32_t Client::GetIp() const
{
    if (protoGame_ && protoGame_->IsConnected())
        return protoGame_->GetIp();
    return 0;
}

int64_t Client::GetClockDiff() const
{
    if (protoGame_)
        return protoGame_->GetClockDiff();
    return 0;
}

void Client::ChangeMap(const std::string& mapUuid)
{
    if (state_ == ClientState::World)
        protoGame_->ChangeMap(mapUuid);
}

void Client::GetMailHeaders()
{
    if (state_ == ClientState::World)
        protoGame_->GetMailHeaders();
}

void Client::GetMail(const std::string& mailUuid)
{
    if (state_ == ClientState::World)
        protoGame_->GetMail(mailUuid);
}

void Client::GetInventory()
{
    if (state_ == ClientState::World)
        protoGame_->GetInventory();
}

void Client::InventoryStoreItem(uint16_t pos)
{
    if (state_ == ClientState::World)
        protoGame_->InventoryStoreItem(pos);
}

void Client::InventoryDestroyItem(uint16_t pos)
{
    if (state_ == ClientState::World)
        protoGame_->InventoryDestroyItem(pos);
}

void Client::InventoryDropItem(uint16_t pos)
{
    if (state_ == ClientState::World)
        protoGame_->InventoryDropItem(pos);
}

void Client::GetChest()
{
    if (state_ == ClientState::World)
        protoGame_->GetChest();
}

void Client::ChestDestroyItem(uint16_t pos)
{
    if (state_ == ClientState::World)
        protoGame_->ChestDestroyItem(pos);
}

void Client::DeleteMail(const std::string& mailUuid)
{
    if (state_ == ClientState::World)
        protoGame_->DeleteMail(mailUuid);
}

void Client::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    if (state_ == ClientState::World)
        protoGame_->SendMail(recipient, subject, body);
}

void Client::GetPlayerInfoByName(const std::string& name)
{
    if (state_ == ClientState::World)
        protoGame_->GetPlayerInfoByName(name);
}

void Client::GetPlayerInfoByAccount(const std::string& accountUuid)
{
    if (state_ == ClientState::World)
        protoGame_->GetPlayerInfoByAccount(accountUuid);
}

void Client::Move(uint8_t direction)
{
    if (state_ == ClientState::World)
        protoGame_->Move(direction);
}

void Client::Turn(uint8_t direction)
{
    if (state_ == ClientState::World)
        protoGame_->Turn(direction);
}

void Client::SetDirection(float rad)
{
    if (state_ == ClientState::World)
        protoGame_->SetDirection(rad);
}

void Client::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == ClientState::World)
        protoGame_->ClickObject(sourceId, targetId);
}

void Client::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == ClientState::World)
        protoGame_->SelectObject(sourceId, targetId);
}

void Client::FollowObject(uint32_t targetId, bool ping)
{
    if (state_ == ClientState::World)
        protoGame_->Follow(targetId, ping);
}

void Client::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    if (state_ == ClientState::World)
        protoGame_->Command(type, data);
}

void Client::GotoPos(const Vec3& pos)
{
    if (state_ == ClientState::World)
        protoGame_->GotoPos(pos);
}

void Client::PartyInvitePlayer(uint32_t targetId)
{
    if (state_ == ClientState::World)
        protoGame_->PartyInvitePlayer(targetId);
}

void Client::PartyKickPlayer(uint32_t targetId)
{
    if (state_ == ClientState::World)
        protoGame_->PartyKickPlayer(targetId);
}

void Client::PartyAcceptInvite(uint32_t inviterId)
{
    if (state_ == ClientState::World)
        protoGame_->PartyAcceptInvite(inviterId);
}

void Client::PartyRejectInvite(uint32_t inviterId)
{
    if (state_ == ClientState::World)
        protoGame_->PartyRejectInvite(inviterId);
}

void Client::PartyGetMembers(uint32_t partyId)
{
    if (state_ == ClientState::World)
        protoGame_->PartyGetMembers(partyId);
}

void Client::PartyLeave()
{
    if (state_ == ClientState::World)
        protoGame_->PartyLeave();
}

void Client::UseSkill(uint32_t index, bool ping)
{
    if (state_ == ClientState::World)
        protoGame_->UseSkill(index, ping);
}

void Client::Attack(bool ping)
{
    if (state_ == ClientState::World)
        protoGame_->Attack(ping);
}

void Client::QueueMatch()
{
    if (state_ == ClientState::World)
        protoGame_->QueueMatch();
}

void Client::UnqueueMatch()
{
    if (state_ == ClientState::World)
        protoGame_->UnqueueMatch();
}

void Client::AddFriend(const std::string& name, AB::Entities::FriendRelation relation)
{
    if (state_ == ClientState::World)
        protoGame_->AddFriend(name, relation);
}

void Client::RemoveFriend(const std::string& accountUuid)
{
    if (state_ == ClientState::World)
        protoGame_->RemoveFriend(accountUuid);
}

void Client::UpdateFriendList()
{
    if (state_ == ClientState::World)
        protoGame_->UpdateFriendList();
}

void Client::Cancel()
{
    if (state_ == ClientState::World)
        protoGame_->Cancel();
}

void Client::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    if (state_ == ClientState::World)
        protoGame_->SetPlayerState(newState);
}

}
