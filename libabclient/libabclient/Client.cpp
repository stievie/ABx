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

void Client::OnAccountCreated()
{
    receiver_.OnAccountCreated();
}

void Client::OnPlayerCreated(const std::string& uuid, const std::string& mapUuid)
{
    receiver_.OnPlayerCreated(uuid, mapUuid);
}

void Client::OnResourceChanged(int64_t updateTick, uint32_t id,
    AB::GameProtocol::ResourceType resType, int16_t value)
{
    receiver_.OnResourceChanged(updateTick, id, resType, value);
}

void Client::OnDialogTrigger(int64_t updateTick, uint32_t dialogId)
{
    receiver_.OnDialogTrigger(updateTick, dialogId);
}

void Client::OnPlayerInfo(int64_t updateTick, const RelatedAccount& player)
{
    receiver_.OnPlayerInfo(updateTick, player);
}

void Client::OnFriendList(int64_t updateTick, const std::vector<std::string>& list)
{
    receiver_.OnFriendList(updateTick, list);
}

void Client::OnFriendAdded(int64_t updateTick, const std::string& accountUuid, RelatedAccount::Relation relation)
{
    receiver_.OnFriendAdded(updateTick, accountUuid, relation);
}

void Client::OnFriendRemoved(int64_t updateTick, const std::string& accountUuid, RelatedAccount::Relation relation)
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

void Client::OnQuestSelectionDialogTrigger(int64_t updateTick, const std::set<uint32_t>& quests)
{
    receiver_.OnQuestSelectionDialogTrigger(updateTick, quests);
}

void Client::OnQuestDialogTrigger(int64_t updateTick, uint32_t questIndex)
{
    receiver_.OnQuestDialogTrigger(updateTick, questIndex);
}

void Client::OnNpcHasQuest(int64_t updateTick, uint32_t npcId, bool hasQuest)
{
    receiver_.OnNpcHasQuest(updateTick, npcId, hasQuest);
}

void Client::OnQuestDeleted(int64_t updateTick, uint32_t index, bool deleted)
{
    receiver_.OnQuestDeleted(updateTick, index, deleted);
}

void Client::OnQuestRewarded(int64_t updateTick, uint32_t index, bool rewarded)
{
    receiver_.OnQuestRewarded(updateTick, index, rewarded);
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
    pings_.Enqueue(lastPing);
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

void Client::GetPlayerInfoByName(const std::string& name, uint32_t fields)
{
    if (state_ == ClientState::World)
        protoGame_->GetPlayerInfoByName(name, fields);
}

void Client::GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields)
{
    if (state_ == ClientState::World)
        protoGame_->GetPlayerInfoByAccount(accountUuid, fields);
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

void Client::SetOnlineStatus(RelatedAccount::Status status)
{
    if (state_ == ClientState::World)
        protoGame_->SetOnlineStatus(status);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerJoined& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerLeft& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChangeInstance& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::EnterWorld& packet)
{
    state_ = ClientState::World;
    mapUuid_ = packet.mapUuid;

    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerAutorun& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawn& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawnExisting& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::MailHeaders& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::MailComplete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDespawn& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPosUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpeedChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryContent& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemDelete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestContent& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemDelete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectRotationUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectTargetSelected& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectStateChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::GameError& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillFailure& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectUseSkill& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillSuccess& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectAttackFailure& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPingTarget& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectAdded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDamaged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectHealed& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectProgress& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDroppedItem& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetPosition& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerMessage& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChatMessage& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerInvited& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerAdded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyInviteRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyResigned& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyDefeated& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyMembersInfo& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

}
